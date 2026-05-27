/*
 * ESP32-C6-LCD-1.47 — Example app on top of the framework.
 *
 * Everything board-specific lives in components/. This file is pure app
 * glue: register the callbacks you care about, then app_run() takes over.
 *
 * The example below mirrors the previous MQTT notification behavior:
 *   - any MQTT message on the chosen topic → render as a notification +
 *     flash the LED green
 *   - online/offline → update the status line
 *
 * Replace the body of on_mqtt() / on_net() with your own logic, or drop
 * them entirely if you don't need MQTT or network awareness.
 *
 * To disable MQTT entirely, set CONFIG_APP_MQTT_ENABLED=n in menuconfig
 * and remove the calls below — the framework is happy without it.
 */

#include "app.h"
#include "esp_log.h"

static const char *TAG = "app_main";

#define MQTT_BROKER_URI  "mqtt://broker.emqx.io"
#define MQTT_TOPIC       "devarshi/esp32c6/notify"

static void on_net(app_net_state_t state, void *ctx)
{
    (void)ctx;
    app_display_show_status(state == APP_NET_UP ? "Online" : "Offline");
}

#ifdef CONFIG_APP_MQTT_ENABLED
static void on_mqtt(const app_mqtt_msg_t *msg, void *ctx)
{
    (void)ctx;
    char title[32] = { 0 };
    char body[160] = { 0 };
    snprintf(title, sizeof(title), "%.*s", msg->topic_len, msg->topic);
    snprintf(body,  sizeof(body),  "%.*s", msg->data_len,  msg->data);
    app_display_notify(title, body);
    app_led_flash(0, 64, 0, 500);
}
#endif

void app_main(void)
{
    ESP_LOGI(TAG, "Starting");

    ESP_ERROR_CHECK(app_init());

    app_on_net_state(on_net, NULL);

#ifdef CONFIG_APP_MQTT_ENABLED
    app_mqtt_subscribe(MQTT_TOPIC, on_mqtt, NULL);
    app_mqtt_init(MQTT_BROKER_URI);
#endif

    app_run();   /* never returns */
}
