/*
 * app_mqtt implementation.
 *
 * Wraps esp-mqtt. Holds:
 *   - One esp_mqtt_client_handle_t.
 *   - A static array of registered subscriptions (topic + callback).
 *
 * Subscription flow:
 *   - app_mqtt_subscribe() always records the (topic, cb) tuple in the table.
 *   - If the client is currently CONNECTED, the topic is also subscribed
 *     immediately so callers don't need to wait for an event.
 *   - On every CONNECTED event, the handler walks the table and re-subscribes
 *     all topics. This makes reconnects automatic.
 *
 * Dispatch:
 *   - On DATA events the topic is matched against subscriptions using a
 *     simple MQTT-aware compare (handles '+' single-level and '#' multi-level
 *     wildcards). Each matching subscription's callback is invoked once.
 */

#include "app_mqtt.h"

#include <string.h>
#include "mqtt_client.h"
#include "esp_log.h"

static const char *TAG = "app_mqtt";

#ifndef CONFIG_APP_MQTT_MAX_SUBSCRIPTIONS
#define CONFIG_APP_MQTT_MAX_SUBSCRIPTIONS 8
#endif

typedef struct {
    const char    *topic;     /* owned by caller, must outlive subscription */
    app_mqtt_cb_t  cb;
    void          *ctx;
    bool           in_use;
} sub_t;

static sub_t                    s_subs[CONFIG_APP_MQTT_MAX_SUBSCRIPTIONS];
static esp_mqtt_client_handle_t s_client;
static bool                     s_connected;

/* ---- topic matching ---------------------------------------------------- */

/*
 * Match a fixed message topic (no wildcards, length-bounded — esp-mqtt passes
 * non-NUL-terminated topic buffers) against a subscription filter that MAY
 * contain MQTT wildcards.
 *
 *   '+' matches exactly one level
 *   '#' matches everything from that point (must be the last char)
 */
static bool topic_match(const char *filter, const char *topic, int topic_len)
{
    int ti = 0;
    while (*filter) {
        if (*filter == '#') {
            return true;
        }
        if (*filter == '+') {
            /* Consume one whole topic level. */
            while (ti < topic_len && topic[ti] != '/') ti++;
            filter++;
            if (*filter == '/' && ti < topic_len && topic[ti] == '/') {
                filter++; ti++;
            } else if (*filter == '\0' && ti == topic_len) {
                return true;
            } else if (*filter == '\0' || ti == topic_len) {
                return false;
            }
            continue;
        }
        if (ti >= topic_len || *filter != topic[ti]) return false;
        filter++; ti++;
    }
    return ti == topic_len;
}

/* ---- event handler ----------------------------------------------------- */

static void on_mqtt_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg; (void)base;
    esp_mqtt_event_handle_t event = data;

    switch ((esp_mqtt_event_id_t)id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "connected");
        s_connected = true;
        /* (Re-)subscribe everything in the table. */
        for (int i = 0; i < CONFIG_APP_MQTT_MAX_SUBSCRIPTIONS; i++) {
            if (s_subs[i].in_use) {
                esp_mqtt_client_subscribe(event->client, s_subs[i].topic, 0);
            }
        }
        break;

    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGW(TAG, "disconnected");
        s_connected = false;
        break;

    case MQTT_EVENT_DATA: {
        app_mqtt_msg_t msg = {
            .topic     = event->topic,
            .topic_len = event->topic_len,
            .data      = event->data,
            .data_len  = event->data_len,
        };
        for (int i = 0; i < CONFIG_APP_MQTT_MAX_SUBSCRIPTIONS; i++) {
            if (s_subs[i].in_use &&
                topic_match(s_subs[i].topic, event->topic, event->topic_len)) {
                s_subs[i].cb(&msg, s_subs[i].ctx);
            }
        }
        break;
    }

    case MQTT_EVENT_ERROR:
        ESP_LOGE(TAG, "mqtt error");
        break;

    default:
        break;
    }
}

/* ---- public API -------------------------------------------------------- */

esp_err_t app_mqtt_init(const char *broker_uri)
{
    if (!broker_uri || !broker_uri[0]) return ESP_ERR_INVALID_ARG;
    if (s_client) return ESP_OK;  /* idempotent */

    esp_mqtt_client_config_t cfg = {
        .broker.address.uri = broker_uri,
    };
    s_client = esp_mqtt_client_init(&cfg);
    if (!s_client) return ESP_FAIL;

    esp_err_t err = esp_mqtt_client_register_event(
        s_client, ESP_EVENT_ANY_ID, on_mqtt_event, NULL);
    if (err != ESP_OK) return err;

    return esp_mqtt_client_start(s_client);
}

esp_err_t app_mqtt_subscribe(const char *topic, app_mqtt_cb_t cb, void *ctx)
{
    if (!topic || !cb) return ESP_ERR_INVALID_ARG;

    for (int i = 0; i < CONFIG_APP_MQTT_MAX_SUBSCRIPTIONS; i++) {
        if (!s_subs[i].in_use) {
            s_subs[i].topic  = topic;
            s_subs[i].cb     = cb;
            s_subs[i].ctx    = ctx;
            s_subs[i].in_use = true;
            if (s_connected && s_client) {
                esp_mqtt_client_subscribe(s_client, topic, 0);
            }
            return ESP_OK;
        }
    }
    ESP_LOGE(TAG, "subscription table full (max %d)",
             CONFIG_APP_MQTT_MAX_SUBSCRIPTIONS);
    return ESP_ERR_NO_MEM;
}

esp_err_t app_mqtt_publish(const char *topic, const void *payload, size_t len,
                           int qos, bool retain)
{
    if (!s_client || !s_connected) return ESP_ERR_INVALID_STATE;
    int msg_id = esp_mqtt_client_publish(s_client, topic, payload, (int)len, qos, retain);
    return (msg_id < 0) ? ESP_FAIL : ESP_OK;
}
