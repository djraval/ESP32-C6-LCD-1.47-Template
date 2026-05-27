/*
 * ESP32-C6-LCD-1.47 — MQTT Notification Display
 *
 * Three framework components keep this file thin:
 *   board_hal — display, backlight, RGB LED (incl. led_hal_flash)
 *   app_ui    — LVGL screen + thread-safe notification queue
 *   net_hal   — WiFi STA (multi-cred scan/remember) + esp-mqtt client
 *
 * The only app-specific logic lives in on_mqtt_message() below: turn an
 * incoming MQTT payload into a UI notification and flash the LED. Forks that
 * want a different app behavior should mostly edit this file.
 */

#include <stdio.h>
#include "esp_log.h"
#include "display_hal.h"
#include "led_hal.h"
#include "ui_manager.h"
#include "net_hal.h"

static const char *TAG = "ESP32C6-NOTIFY";

static void on_mqtt_message(const net_hal_mqtt_msg_t *msg, void *arg)
{
    (void)arg;
    ui_notification_t n = { 0 };
    snprintf(n.title, sizeof(n.title), "%.*s", msg->topic_len, msg->topic);
    snprintf(n.body,  sizeof(n.body),  "%.*s", msg->data_len,  msg->data);
    ui_manager_post_notification(&n);
    led_hal_flash(0, 64, 0, 500);
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C6 MQTT notification display starting...");

    ESP_ERROR_CHECK(display_hal_init());
    ESP_ERROR_CHECK(led_hal_init());
    ESP_ERROR_CHECK(ui_manager_init());

    // Network is best-effort: if WiFi fails to associate we still want the UI
    // up so the user can see the failure and power-cycle.
    esp_err_t net_err = net_hal_init(&(net_hal_config_t){
        .on_mqtt_data = on_mqtt_message,
    });
    if (net_err != ESP_OK) {
        ESP_LOGE(TAG, "Network init failed: %s — continuing without MQTT",
                 esp_err_to_name(net_err));
    }

    ui_manager_run();
}
