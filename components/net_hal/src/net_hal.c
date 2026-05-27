/*
 * Network HAL implementation — WiFi station + esp-mqtt client.
 *
 * Connection flow:
 *   1. NVS init (also used by the WiFi driver).
 *   2. Build credential list from runtime override or Kconfig slots.
 *   3. Fast path: try the SSID that worked last boot (recall from NVS).
 *   4. Fallback: active scan, sort by RSSI, try each in-range known cred.
 *   5. Save the SSID we ended up connecting to.
 *   6. Start MQTT, subscribe, dispatch incoming messages via callback.
 */

#include "net_hal.h"

#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "mqtt_client.h"

#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include <stdlib.h>
#include <string.h>

static const char *TAG = "NET_HAL";

#define WIFI_CONNECTED_BIT       BIT0
#define WIFI_FAIL_BIT            BIT1
#define WIFI_CONNECT_TIMEOUT_MS  8000   /* per-credential attempt */

#define NVS_NS                   "net_hal"
#define NVS_KEY_LAST_SSID        "last_ssid"

#define MAX_KCONFIG_CREDS        3
#define MAX_SSID_BUF             33     /* 32 chars + NUL */

static EventGroupHandle_t s_wifi_event_group;
static esp_mqtt_client_handle_t s_mqtt_client;

static net_hal_mqtt_data_cb_t s_on_mqtt_data;
static void *s_on_mqtt_data_arg;

/* ---- WiFi event handling ------------------------------------------------ */

static void wifi_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    if (base == WIFI_EVENT && id == WIFI_EVENT_STA_DISCONNECTED) {
        xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = (ip_event_got_ip_t *)data;
        ESP_LOGI(TAG, "Got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/* ---- NVS helpers (remember the last successful SSID) -------------------- */

static void remember_ssid(const char *ssid)
{
    nvs_handle_t h;
    if (nvs_open(NVS_NS, NVS_READWRITE, &h) != ESP_OK) {
        return;
    }
    nvs_set_str(h, NVS_KEY_LAST_SSID, ssid);
    nvs_commit(h);
    nvs_close(h);
}

static esp_err_t recall_ssid(char *out, size_t cap)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(NVS_NS, NVS_READONLY, &h);
    if (err != ESP_OK) {
        return err;
    }
    size_t len = cap;
    err = nvs_get_str(h, NVS_KEY_LAST_SSID, out, &len);
    nvs_close(h);
    return err;
}

/* ---- Connection logic --------------------------------------------------- */

static esp_err_t try_connect(const net_hal_wifi_cred_t *cred)
{
    wifi_config_t cfg = { 0 };
    strncpy((char *)cfg.sta.ssid, cred->ssid, sizeof(cfg.sta.ssid) - 1);
    if (cred->password && cred->password[0] != '\0') {
        strncpy((char *)cfg.sta.password, cred->password, sizeof(cfg.sta.password) - 1);
        cfg.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    } else {
        cfg.sta.threshold.authmode = WIFI_AUTH_OPEN;
    }

    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &cfg);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_set_config: %s", esp_err_to_name(err));
        return err;
    }

    /* Clear stale bits from any previous attempt's trailing disconnect. */
    xEventGroupClearBits(s_wifi_event_group, WIFI_CONNECTED_BIT | WIFI_FAIL_BIT);

    err = esp_wifi_connect();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_connect: %s", esp_err_to_name(err));
        return err;
    }

    EventBits_t bits = xEventGroupWaitBits(
        s_wifi_event_group,
        WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
        pdFALSE, pdFALSE,
        pdMS_TO_TICKS(WIFI_CONNECT_TIMEOUT_MS));

    if (bits & WIFI_CONNECTED_BIT) {
        return ESP_OK;
    }

    if (bits & WIFI_FAIL_BIT) {
        ESP_LOGW(TAG, "Could not connect to '%s' (auth/assoc failed)", cred->ssid);
    } else {
        ESP_LOGW(TAG, "Could not connect to '%s' (no event in %d ms)",
                 cred->ssid, WIFI_CONNECT_TIMEOUT_MS);
    }
    esp_wifi_disconnect();
    return ESP_FAIL;
}

static int cmp_rssi_desc(const void *a, const void *b)
{
    const wifi_ap_record_t *ra = a;
    const wifi_ap_record_t *rb = b;
    /* Strongest first (RSSI is a negative dBm value). */
    return rb->rssi - ra->rssi;
}

static esp_err_t connect_best_wifi(const net_hal_wifi_cred_t *creds, size_t count)
{
    if (count == 0) {
        ESP_LOGE(TAG, "No WiFi credentials configured (set CONFIG_NOTIFY_WIFI_SSID or pass cfg->wifi_creds)");
        return ESP_ERR_INVALID_ARG;
    }

    /* Always scan first — the WiFi driver associates more reliably with a
     * fresh scan in hand, and the scan gives us RSSIs for ranking. */
    ESP_LOGI(TAG, "Scanning for known networks...");
    wifi_scan_config_t scan_cfg = { 0 };
    esp_err_t err = esp_wifi_scan_start(&scan_cfg, true /* blocking */);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_wifi_scan_start: %s", esp_err_to_name(err));
        return err;
    }

    uint16_t ap_num = 0;
    esp_wifi_scan_get_ap_num(&ap_num);
    if (ap_num == 0) {
        ESP_LOGW(TAG, "Scan returned 0 APs");
        return ESP_ERR_NOT_FOUND;
    }

    wifi_ap_record_t *records = calloc(ap_num, sizeof(*records));
    if (!records) {
        return ESP_ERR_NO_MEM;
    }
    err = esp_wifi_scan_get_ap_records(&ap_num, records);
    if (err != ESP_OK) {
        free(records);
        return err;
    }
    qsort(records, ap_num, sizeof(*records), cmp_rssi_desc);

    /* If we remember a previously-successful SSID and it's in range AND in
     * the cred list, try it first — even if another known network has a
     * stronger signal right now. */
    char remembered[MAX_SSID_BUF] = "";
    bool have_remembered = (recall_ssid(remembered, sizeof(remembered)) == ESP_OK
                            && remembered[0] != '\0');

    if (have_remembered) {
        for (uint16_t i = 0; i < ap_num; i++) {
            if (strcmp((const char *)records[i].ssid, remembered) != 0) continue;
            for (size_t c = 0; c < count; c++) {
                if (creds[c].ssid && strcmp(remembered, creds[c].ssid) == 0) {
                    ESP_LOGI(TAG, "Trying remembered SSID '%s' (RSSI=%d)",
                             remembered, records[i].rssi);
                    if (try_connect(&creds[c]) == ESP_OK) {
                        free(records);
                        return ESP_OK;
                    }
                    ESP_LOGW(TAG, "Remembered SSID didn't connect; trying others");
                    break;
                }
            }
            break;
        }
    }

    /* Try each remaining in-range known SSID by descending RSSI. */
    for (uint16_t i = 0; i < ap_num; i++) {
        const char *seen = (const char *)records[i].ssid;
        if (have_remembered && strcmp(seen, remembered) == 0) {
            continue; /* already tried above */
        }
        for (size_t c = 0; c < count; c++) {
            if (creds[c].ssid && strcmp(seen, creds[c].ssid) == 0) {
                ESP_LOGI(TAG, "Trying '%s' (RSSI=%d)", seen, records[i].rssi);
                if (try_connect(&creds[c]) == ESP_OK) {
                    remember_ssid(creds[c].ssid);
                    free(records);
                    return ESP_OK;
                }
                break; /* each SSID only appears once in cred list */
            }
        }
    }

    free(records);
    ESP_LOGE(TAG, "No known network in range (scanned %u APs)", ap_num);
    return ESP_ERR_NOT_FOUND;
}

static esp_err_t wifi_bringup(const net_hal_wifi_cred_t *creds, size_t count)
{
    s_wifi_event_group = xEventGroupCreate();
    if (!s_wifi_event_group) {
        return ESP_ERR_NO_MEM;
    }

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t init_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&init_cfg));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT, ESP_EVENT_ANY_ID, &wifi_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT, IP_EVENT_STA_GOT_IP, &wifi_event_handler, NULL, NULL));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());

    return connect_best_wifi(creds, count);
}

/* ---- MQTT --------------------------------------------------------------- */

static void mqtt_event_handler(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    esp_mqtt_event_handle_t event = data;
    switch ((esp_mqtt_event_id_t)id) {
        case MQTT_EVENT_CONNECTED:
            ESP_LOGI(TAG, "MQTT connected; subscribing to %s", CONFIG_NOTIFY_MQTT_TOPIC);
            esp_mqtt_client_subscribe(event->client, CONFIG_NOTIFY_MQTT_TOPIC, 0);
            break;
        case MQTT_EVENT_DISCONNECTED:
            ESP_LOGW(TAG, "MQTT disconnected");
            break;
        case MQTT_EVENT_DATA:
            ESP_LOGI(TAG, "MQTT msg on %.*s (%d bytes)",
                     event->topic_len, event->topic, event->data_len);
            if (s_on_mqtt_data) {
                net_hal_mqtt_msg_t msg = {
                    .topic     = event->topic,
                    .topic_len = event->topic_len,
                    .data      = event->data,
                    .data_len  = event->data_len,
                };
                s_on_mqtt_data(&msg, s_on_mqtt_data_arg);
            }
            break;
        case MQTT_EVENT_ERROR:
            ESP_LOGE(TAG, "MQTT error");
            break;
        default:
            break;
    }
}

static esp_err_t mqtt_start(void)
{
    esp_mqtt_client_config_t mqtt_cfg = {
        .broker.address.uri = CONFIG_NOTIFY_MQTT_BROKER_URI,
    };
    s_mqtt_client = esp_mqtt_client_init(&mqtt_cfg);
    if (!s_mqtt_client) {
        return ESP_FAIL;
    }
    ESP_ERROR_CHECK(esp_mqtt_client_register_event(
        s_mqtt_client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL));
    return esp_mqtt_client_start(s_mqtt_client);
}

/* ---- Kconfig slot harvesting ------------------------------------------- */

static size_t build_kconfig_creds(net_hal_wifi_cred_t *out, size_t cap)
{
    static const struct {
        const char *ssid;
        const char *password;
    } slots[] = {
        { CONFIG_NOTIFY_WIFI_SSID,   CONFIG_NOTIFY_WIFI_PASSWORD   },
        { CONFIG_NOTIFY_WIFI_SSID_2, CONFIG_NOTIFY_WIFI_PASSWORD_2 },
        { CONFIG_NOTIFY_WIFI_SSID_3, CONFIG_NOTIFY_WIFI_PASSWORD_3 },
    };
    size_t n = 0;
    for (size_t i = 0; i < sizeof(slots) / sizeof(slots[0]) && n < cap; i++) {
        if (slots[i].ssid && slots[i].ssid[0] != '\0') {
            out[n].ssid     = slots[i].ssid;
            out[n].password = slots[i].password;
            n++;
        }
    }
    return n;
}

/* ---- Public entry point ------------------------------------------------- */

esp_err_t net_hal_init(const net_hal_config_t *cfg)
{
    ESP_LOGI(TAG, "Initializing network");

    if (cfg) {
        s_on_mqtt_data     = cfg->on_mqtt_data;
        s_on_mqtt_data_arg = cfg->user_arg;
    }

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    net_hal_wifi_cred_t kconfig_creds[MAX_KCONFIG_CREDS];
    const net_hal_wifi_cred_t *creds;
    size_t count;
    if (cfg && cfg->wifi_creds && cfg->wifi_cred_count > 0) {
        creds = cfg->wifi_creds;
        count = cfg->wifi_cred_count;
    } else {
        count = build_kconfig_creds(kconfig_creds, MAX_KCONFIG_CREDS);
        creds = kconfig_creds;
    }

    ret = wifi_bringup(creds, count);
    if (ret != ESP_OK) {
        return ret;
    }

    return mqtt_start();
}
