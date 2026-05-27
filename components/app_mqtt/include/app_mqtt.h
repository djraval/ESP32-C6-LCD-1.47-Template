/*
 * app_mqtt — optional MQTT client service.
 *
 * Compile-time opt-in via CONFIG_APP_MQTT_ENABLED. When disabled, the public
 * functions are still declared but the implementation is not linked — the
 * umbrella app.h hides them behind an #ifdef so app code that uses MQTT
 * fails fast at build time.
 *
 * Usage:
 *
 *     app_mqtt_init("mqtt://broker.emqx.io");
 *     app_mqtt_subscribe("home/sensor/#", on_msg, NULL);
 *     app_mqtt_publish("home/status", "alive", 5, 0, false);
 *
 * Subscriptions registered before MQTT connects are stored and replayed
 * on each CONNECT event so reconnects re-subscribe automatically.
 */

#pragma once

#include <stdbool.h>
#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    const char *topic;     /* NOT null-terminated; use topic_len */
    int         topic_len;
    const char *data;
    int         data_len;
} app_mqtt_msg_t;

typedef void (*app_mqtt_cb_t)(const app_mqtt_msg_t *msg, void *ctx);

/**
 * @brief Start the MQTT client against @p broker_uri.
 *
 * Safe to call before the network is up — esp-mqtt will connect on its own
 * once WiFi is ready and reconnect after drops.
 *
 * @param broker_uri Full URI including scheme: "mqtt://host:port" or
 *                   "mqtts://..." for TLS. Must remain valid for the
 *                   process lifetime (typically a string literal).
 */
esp_err_t app_mqtt_init(const char *broker_uri);

/**
 * @brief Subscribe to @p topic; invoke @p cb for each matching message.
 *
 * May be called before or after app_mqtt_init() / connection. Subscriptions
 * are remembered and replayed on every (re)connect.
 *
 * @return ESP_OK on success, ESP_ERR_NO_MEM if the subscription table is full.
 */
esp_err_t app_mqtt_subscribe(const char *topic, app_mqtt_cb_t cb, void *ctx);

/**
 * @brief Publish a message. The client must be connected; if not, the
 *        message is dropped and ESP_ERR_INVALID_STATE is returned.
 */
esp_err_t app_mqtt_publish(const char *topic, const void *payload, size_t len,
                           int qos, bool retain);

#ifdef __cplusplus
}
#endif
