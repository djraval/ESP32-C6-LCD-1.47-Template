/*
 * Network HAL — WiFi station + MQTT client.
 *
 * Brings up WiFi against one of several known credentials (scanned and chosen
 * by signal strength, with the last successful SSID remembered in NVS for a
 * fast-path on subsequent boots), then connects to an MQTT broker and
 * subscribes to a topic. Incoming messages are delivered to the caller via a
 * user-supplied callback so this component stays independent of any UI layer.
 *
 * Credentials default to Kconfig slots CONFIG_NOTIFY_WIFI_SSID(_2/_3); the
 * caller can override at runtime by passing a list through net_hal_config_t.
 */

#pragma once

#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief A single WiFi credential (network name + password).
 *
 * Pointed-to strings must remain valid for the lifetime of net_hal_init().
 * `password` may be NULL or "" for open networks.
 */
typedef struct {
    const char *ssid;     /* null-terminated; empty means "unused slot" */
    const char *password; /* null-terminated; "" or NULL for open networks */
} net_hal_wifi_cred_t;

/**
 * @brief A received MQTT message.
 *
 * Buffers are owned by esp-mqtt and only valid for the duration of the
 * callback call — copy what you need before returning. Topic and data are
 * NOT null-terminated; use the matching length fields.
 */
typedef struct {
    const char *topic;
    int   topic_len;
    const char *data;
    int   data_len;
} net_hal_mqtt_msg_t;

/**
 * @brief Called from the MQTT task each time a message arrives on the
 *        subscribed topic.
 */
typedef void (*net_hal_mqtt_data_cb_t)(const net_hal_mqtt_msg_t *msg, void *user_arg);

/**
 * @brief Configuration passed to net_hal_init().
 *
 * Any field left zero/NULL falls back to a sensible default:
 *   - on_mqtt_data NULL → received messages are silently dropped.
 *   - wifi_creds   NULL → Kconfig slots 1..3 are used.
 */
typedef struct {
    net_hal_mqtt_data_cb_t on_mqtt_data;
    void *user_arg;                          /* opaque, passed back to the callback */
    const net_hal_wifi_cred_t *wifi_creds;
    size_t wifi_cred_count;
} net_hal_config_t;

/**
 * @brief Bring up WiFi (multi-cred scan + remember-last) and MQTT.
 *
 * Blocks until WiFi associates with one of the configured networks and gets
 * an IP, or until all credentials are exhausted (~8 s per attempt). Returns
 * ESP_ERR_NOT_FOUND if no known network is in range. MQTT itself runs
 * asynchronously after this returns.
 *
 * @param cfg Configuration, may be NULL for "all defaults".
 */
esp_err_t net_hal_init(const net_hal_config_t *cfg);

#ifdef __cplusplus
}
#endif
