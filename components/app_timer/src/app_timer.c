/*
 * app_timer — esp_timer with a worker task.
 *
 * esp_timer fires its callbacks in a high-priority "esp_timer" task context;
 * doing heavy work there blocks other timers. We post a small work item to a
 * dedicated worker so user callbacks can take their time.
 */

#include "app_timer.h"

#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "sdkconfig.h"

static const char *TAG = "app_timer";

struct app_timer {
    esp_timer_handle_t et;
    app_timer_cb_t     cb;
    void              *ctx;
};

typedef struct {
    app_timer_cb_t cb;
    void          *ctx;
} work_item_t;

static QueueHandle_t s_q;
static TaskHandle_t  s_worker;

static void worker_task(void *arg)
{
    (void)arg;
    work_item_t w;
    for (;;) {
        if (xQueueReceive(s_q, &w, portMAX_DELAY) == pdTRUE && w.cb) {
            w.cb(w.ctx);
        }
    }
}

static esp_err_t lazy_init(void)
{
    if (s_q) return ESP_OK;
    s_q = xQueueCreate(CONFIG_APP_TIMER_QUEUE_LEN, sizeof(work_item_t));
    if (!s_q) return ESP_ERR_NO_MEM;
    BaseType_t ok = xTaskCreate(worker_task, "app_timer",
                                CONFIG_APP_TIMER_WORKER_STACK,
                                NULL, 5, &s_worker);
    if (ok != pdPASS) {
        vQueueDelete(s_q);
        s_q = NULL;
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

static void et_trampoline(void *arg)
{
    /* esp_timer ISR-ish context — bounce to worker. */
    struct app_timer *t = arg;
    work_item_t w = { .cb = t->cb, .ctx = t->ctx };
    (void)xQueueSend(s_q, &w, 0);
}

esp_err_t app_timer_every(uint32_t period_ms, app_timer_cb_t cb, void *ctx,
                          app_timer_handle_t *out)
{
    if (!cb || period_ms == 0) return ESP_ERR_INVALID_ARG;
    esp_err_t err = lazy_init();
    if (err != ESP_OK) return err;

    struct app_timer *t = calloc(1, sizeof(*t));
    if (!t) return ESP_ERR_NO_MEM;
    t->cb = cb; t->ctx = ctx;

    const esp_timer_create_args_t args = {
        .callback = et_trampoline,
        .arg      = t,
        .name     = "app_timer",
    };
    err = esp_timer_create(&args, &t->et);
    if (err != ESP_OK) { free(t); return err; }
    err = esp_timer_start_periodic(t->et, (uint64_t)period_ms * 1000ULL);
    if (err != ESP_OK) {
        esp_timer_delete(t->et);
        free(t);
        return err;
    }
    if (out) *out = t;
    return ESP_OK;
}

esp_err_t app_timer_cancel(app_timer_handle_t h)
{
    if (!h) return ESP_ERR_INVALID_ARG;
    esp_timer_stop(h->et);
    esp_timer_delete(h->et);
    free(h);
    return ESP_OK;
}

esp_err_t app_defer(app_timer_cb_t cb, void *ctx)
{
    if (!cb) return ESP_ERR_INVALID_ARG;
    esp_err_t err = lazy_init();
    if (err != ESP_OK) return err;
    work_item_t w = { .cb = cb, .ctx = ctx };
    return xQueueSend(s_q, &w, 0) == pdTRUE ? ESP_OK : ESP_ERR_NO_MEM;
}
