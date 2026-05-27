/*
 * app_init / app_run — framework lifecycle.
 *
 * Init order matters:
 *   1. display_hal_init — power up the LCD before anything tries to render
 *   2. led_hal_init     — RGB LED ready for status indication
 *   3. ui_manager_init  — LVGL screen + notification queue
 *   4. net_hal_init     — best-effort WiFi + MQTT (Phase 1)
 *
 * Network failures are non-fatal: the LCD stays usable so the user can see
 * what went wrong and react.
 */

#include "app.h"

#include <stdio.h>

#include "esp_log.h"
#include "esp_timer.h"
#include "driver/gpio.h"
#include "display_hal.h"
#include "led_hal.h"
#include "ui_manager.h"
#include "net_hal.h"
#include "board_config.h"
#include "sdkconfig.h"

static const char *TAG = "app";

#define REPROV_HOLD_MS  3000

/* From app_net.c (private). */
extern void app_net_set_state_internal(app_net_state_t s);

/* ---- BOOT-button long-press: erase creds + reboot ------------------ */

#ifdef CONFIG_APP_BUTTON_ENABLED

static void boot_long_press_cb(app_button_event_t ev, void *ctx)
{
    (void)ev; (void)ctx;
    ESP_LOGW(TAG, "BOOT held %d ms — wiping WiFi credentials", REPROV_HOLD_MS);
    app_display_notify("Reprovisioning", "Wiping creds & rebooting");
    app_led_set(64, 0, 0);
    net_hal_reset_credentials();   /* reboots */
}

static void install_boot_button(void)
{
    app_button_register(CONFIG_APP_BUTTON_BOOT_GPIO,
                        APP_BUTTON_LONG_PRESS,
                        REPROV_HOLD_MS,
                        boot_long_press_cb, NULL);
}

#else  /* fallback: inline GPIO9 handler */

static esp_timer_handle_t s_boot_timer;

static void boot_long_press_cb(void *arg)
{
    (void)arg;
    if (gpio_get_level(PIN_BOOT_BUTTON) == 0) {
        ESP_LOGW(TAG, "BOOT held %d ms — wiping WiFi credentials", REPROV_HOLD_MS);
        app_display_notify("Reprovisioning", "Wiping creds & rebooting");
        app_led_set(64, 0, 0);
        net_hal_reset_credentials();
    }
}

static void IRAM_ATTR boot_isr(void *arg)
{
    (void)arg;
    if (gpio_get_level(PIN_BOOT_BUTTON) == 0) {
        esp_timer_start_once(s_boot_timer, REPROV_HOLD_MS * 1000ULL);
    } else {
        esp_timer_stop(s_boot_timer);
    }
}

static void install_boot_button(void)
{
    const esp_timer_create_args_t targs = {
        .callback = boot_long_press_cb,
        .name     = "boot_long",
    };
    if (esp_timer_create(&targs, &s_boot_timer) != ESP_OK) {
        ESP_LOGE(TAG, "boot timer create failed");
        return;
    }
    gpio_config_t io = {
        .pin_bit_mask = 1ULL << PIN_BOOT_BUTTON,
        .mode         = GPIO_MODE_INPUT,
        .pull_up_en   = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type    = GPIO_INTR_ANYEDGE,
    };
    gpio_config(&io);
    gpio_install_isr_service(0);
    gpio_isr_handler_add(PIN_BOOT_BUTTON, boot_isr, NULL);
}

#endif  /* CONFIG_APP_BUTTON_ENABLED */

/* ---- net_hal event bridge ------------------------------------------ */

static void on_net_event(const net_hal_event_t *ev, void *ctx)
{
    (void)ctx;
    char body[160];

    switch (ev->state) {
    case NET_HAL_PROVISIONING:
        snprintf(body, sizeof(body), "Open ESP BLE Prov app\nName: %s\nPoP:  %s",
                 ev->prov_service_name ? ev->prov_service_name : "?",
                 ev->prov_pop          ? ev->prov_pop          : "?");
        app_display_notify("Provisioning", body);
        app_led_pulse(0, 0, 64, 1000);    /* slow blue pulse */
        app_net_set_state_internal(APP_NET_DOWN);
        break;
    case NET_HAL_CONNECTING:
        app_display_notify("Connecting", "WiFi associating...");
        app_led_pulse(64, 32, 0, 500);    /* fast amber pulse */
        app_net_set_state_internal(APP_NET_DOWN);
        break;
    case NET_HAL_CONNECTED:
        app_display_notify("Online", "Ready");
        app_led_off();
        app_net_set_state_internal(APP_NET_UP);
        break;
    case NET_HAL_DISCONNECTED:
        app_display_show_status("Offline");
        app_led_pulse(64, 0, 0, 1000);
        app_net_set_state_internal(APP_NET_DOWN);
        break;
    case NET_HAL_FAILED:
        app_display_notify("Provisioning failed",
                           "Hold BOOT 3s to retry");
        app_led_set(64, 0, 0);
        app_net_set_state_internal(APP_NET_DOWN);
        break;
    default:
        break;
    }
}

/* ---- public lifecycle ----------------------------------------------- */

esp_err_t app_init(void)
{
    ESP_LOGI(TAG, "app_init");

    esp_err_t err = display_hal_init();
    if (err != ESP_OK) { ESP_LOGE(TAG, "display_hal_init: %s", esp_err_to_name(err)); return err; }

    err = led_hal_init();
    if (err != ESP_OK) { ESP_LOGE(TAG, "led_hal_init: %s", esp_err_to_name(err)); return err; }

    err = ui_manager_init();
    if (err != ESP_OK) { ESP_LOGE(TAG, "ui_manager_init: %s", esp_err_to_name(err)); return err; }

    install_boot_button();

    net_hal_config_t net_cfg = {
        .event_cb  = on_net_event,
        .event_ctx = NULL,
    };
    esp_err_t net_err = net_hal_init(&net_cfg);
    if (net_err != ESP_OK) {
        ESP_LOGE(TAG, "net_hal_init: %s — continuing offline",
                 esp_err_to_name(net_err));
        app_net_set_state_internal(APP_NET_DOWN);
    }
    /* No synchronous "up" here — events from net_hal drive the state. */

    return ESP_OK;
}

void app_run(void)
{
    ui_manager_run();   /* never returns */
}
