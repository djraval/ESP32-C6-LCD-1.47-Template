/*
 * app_button — small GPIO button helper.
 *
 * One ISR handler per registered pin. The ISR drops the event into a queue;
 * a worker task does the debouncing (re-samples after DEBOUNCE_MS) and runs
 * user callbacks. Long-press is implemented with a per-handler esp_timer:
 * armed on debounced press, cancelled on release.
 */

#include "app_button.h"

#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sdkconfig.h"

static const char *TAG = "app_button";

#define MAX_HANDLERS 8

typedef struct {
    int                 gpio;
    app_button_event_t  mode;
    uint32_t            long_press_ms;
    app_button_cb_t     cb;
    void               *ctx;
    esp_timer_handle_t  lp_timer;   /* only for long-press mode */
    bool                last_level_low;
} handler_t;

static handler_t      s_handlers[MAX_HANDLERS];
static int            s_n_handlers;
static QueueHandle_t  s_evt_q;     /* gpio numbers */
static bool           s_isr_installed;

static void IRAM_ATTR gpio_isr(void *arg)
{
    int gpio = (int)(intptr_t)arg;
    BaseType_t hpw = pdFALSE;
    xQueueSendFromISR(s_evt_q, &gpio, &hpw);
    if (hpw) portYIELD_FROM_ISR();
}

static void long_press_cb(void *arg)
{
    handler_t *h = arg;
    if (gpio_get_level(h->gpio) == 0 && h->cb) {
        h->cb(APP_BUTTON_LONG_PRESS, h->ctx);
    }
}

static void worker_task(void *arg)
{
    (void)arg;
    int gpio;
    for (;;) {
        if (xQueueReceive(s_evt_q, &gpio, portMAX_DELAY) != pdTRUE) continue;
        vTaskDelay(pdMS_TO_TICKS(CONFIG_APP_BUTTON_DEBOUNCE_MS));
        bool low = gpio_get_level(gpio) == 0;

        for (int i = 0; i < s_n_handlers; i++) {
            handler_t *h = &s_handlers[i];
            if (h->gpio != gpio) continue;

            if (low && !h->last_level_low) {
                if (h->mode == APP_BUTTON_PRESS && h->cb) {
                    h->cb(APP_BUTTON_PRESS, h->ctx);
                }
                if (h->mode == APP_BUTTON_LONG_PRESS && h->lp_timer) {
                    esp_timer_start_once(h->lp_timer,
                                         (uint64_t)h->long_press_ms * 1000ULL);
                }
            } else if (!low && h->last_level_low) {
                if (h->mode == APP_BUTTON_RELEASE && h->cb) {
                    h->cb(APP_BUTTON_RELEASE, h->ctx);
                }
                if (h->mode == APP_BUTTON_LONG_PRESS && h->lp_timer) {
                    esp_timer_stop(h->lp_timer);
                }
            }
            h->last_level_low = low;
        }
    }
}

static esp_err_t ensure_runtime(void)
{
    if (s_evt_q) return ESP_OK;
    s_evt_q = xQueueCreate(16, sizeof(int));
    if (!s_evt_q) return ESP_ERR_NO_MEM;
    BaseType_t ok = xTaskCreate(worker_task, "app_button", 3072, NULL, 5, NULL);
    if (ok != pdPASS) return ESP_ERR_NO_MEM;
    return ESP_OK;
}

esp_err_t app_button_register(int gpio, app_button_event_t mode,
                              uint32_t long_press_ms,
                              app_button_cb_t cb, void *ctx)
{
    if (!cb || gpio < 0) return ESP_ERR_INVALID_ARG;
    if (mode == APP_BUTTON_LONG_PRESS && long_press_ms == 0) return ESP_ERR_INVALID_ARG;
    if (s_n_handlers >= MAX_HANDLERS) return ESP_ERR_NO_MEM;

    esp_err_t err = ensure_runtime();
    if (err != ESP_OK) return err;

    bool pin_already_configured = false;
    for (int i = 0; i < s_n_handlers; i++) {
        if (s_handlers[i].gpio == gpio) { pin_already_configured = true; break; }
    }

    if (!pin_already_configured) {
        gpio_config_t io = {
            .pin_bit_mask = 1ULL << gpio,
            .mode         = GPIO_MODE_INPUT,
            .pull_up_en   = GPIO_PULLUP_ENABLE,
            .pull_down_en = GPIO_PULLDOWN_DISABLE,
            .intr_type    = GPIO_INTR_ANYEDGE,
        };
        err = gpio_config(&io);
        if (err != ESP_OK) return err;
        if (!s_isr_installed) {
            /* Ignore "already installed" — another component may own it. */
            esp_err_t isr_err = gpio_install_isr_service(0);
            if (isr_err != ESP_OK && isr_err != ESP_ERR_INVALID_STATE) return isr_err;
            s_isr_installed = true;
        }
        gpio_isr_handler_add(gpio, gpio_isr, (void *)(intptr_t)gpio);
    }

    handler_t *h = &s_handlers[s_n_handlers++];
    h->gpio          = gpio;
    h->mode          = mode;
    h->long_press_ms = long_press_ms;
    h->cb            = cb;
    h->ctx           = ctx;
    h->last_level_low = (gpio_get_level(gpio) == 0);

    if (mode == APP_BUTTON_LONG_PRESS) {
        const esp_timer_create_args_t targs = {
            .callback = long_press_cb,
            .arg      = h,
            .name     = "app_btn_lp",
        };
        esp_timer_create(&targs, &h->lp_timer);
    }

    ESP_LOGI(TAG, "registered gpio=%d mode=%d", gpio, mode);
    return ESP_OK;
}
