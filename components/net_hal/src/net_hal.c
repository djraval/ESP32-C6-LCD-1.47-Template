/*
 * Network HAL — WiFi + BLE provisioning.
 *
 * Implementation notes:
 *
 *   - All work happens on a dedicated "net_hal" task. net_hal_init() spawns
 *     it and returns; state changes flow through the user callback.
 *
 *   - On boot we ask wifi_provisioning_manager whether credentials exist.
 *     If not, we advertise over BLE (NimBLE), publish service name +
 *     PoP via the event callback so the UI can display them, and wait for
 *     credentials. wifi_provisioning_manager auto-starts WiFi STA and
 *     connects when credentials arrive.
 *
 *   - Once connected (IP_EVENT_STA_GOT_IP), we transition to CONNECTED and
 *     stop / deinit the provisioning manager to free its resources.
 *
 *   - WIFI_EVENT_STA_DISCONNECTED after a successful connect is treated as
 *     a transient drop; the WiFi driver retries on its own and we stay in
 *     CONNECTING until either GOT_IP fires again or the user resets.
 */

#include "net_hal.h"

#include <string.h>
#include <stdio.h>

#include "esp_log.h"
#include "esp_event.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_mac.h"
#include "esp_system.h"
#include "nvs_flash.h"

#include "wifi_provisioning/manager.h"
#include "wifi_provisioning/scheme_ble.h"

#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

static const char *TAG = "net_hal";

/* Hardcoded for now — exposed through Kconfig in a polish pass. */
#define PROV_NAME_PREFIX "PROV_"
#define PROV_POP         "abcd1234"

#define EVT_GOT_IP       BIT0
#define EVT_PROV_DONE    BIT1
#define EVT_PROV_FAIL    BIT2

static EventGroupHandle_t s_events;
static net_hal_event_cb_t s_user_cb;
static void              *s_user_ctx;
static net_hal_state_t    s_state = NET_HAL_BOOTING;

static char s_service_name[16];

static void emit(net_hal_state_t st, const char *svc, const char *pop)
{
    s_state = st;
    if (!s_user_cb) return;
    net_hal_event_t ev = {
        .state             = st,
        .prov_service_name = svc,
        .prov_pop          = pop,
    };
    s_user_cb(&ev, s_user_ctx);
}

static void build_service_name(char *out, size_t cap)
{
    uint8_t mac[6];
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    snprintf(out, cap, "%s%02X%02X", PROV_NAME_PREFIX, mac[4], mac[5]);
}

/* ---- event handler --------------------------------------------------- */

static void on_event(void *arg, esp_event_base_t base, int32_t id, void *data)
{
    (void)arg;

    if (base == WIFI_PROV_EVENT) {
        switch (id) {
        case WIFI_PROV_START:
            ESP_LOGI(TAG, "BLE provisioning started: %s (PoP=%s)",
                     s_service_name, PROV_POP);
            emit(NET_HAL_PROVISIONING, s_service_name, PROV_POP);
            break;
        case WIFI_PROV_CRED_RECV:
            ESP_LOGI(TAG, "creds received");
            emit(NET_HAL_CONNECTING, NULL, NULL);
            break;
        case WIFI_PROV_CRED_FAIL:
            ESP_LOGW(TAG, "cred verification failed");
            xEventGroupSetBits(s_events, EVT_PROV_FAIL);
            emit(NET_HAL_FAILED, NULL, NULL);
            break;
        case WIFI_PROV_CRED_SUCCESS:
            ESP_LOGI(TAG, "cred verified, associating...");
            break;
        case WIFI_PROV_END:
            ESP_LOGI(TAG, "provisioning service stopping");
            xEventGroupSetBits(s_events, EVT_PROV_DONE);
            wifi_prov_mgr_deinit();
            break;
        default:
            break;
        }
    } else if (base == WIFI_EVENT) {
        if (id == WIFI_EVENT_STA_DISCONNECTED) {
            if (s_state == NET_HAL_CONNECTED) {
                ESP_LOGW(TAG, "STA disconnected; driver will retry");
                emit(NET_HAL_DISCONNECTED, NULL, NULL);
            }
            esp_wifi_connect();
        } else if (id == WIFI_EVENT_STA_START) {
            esp_wifi_connect();
        }
    } else if (base == IP_EVENT && id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t *event = data;
        ESP_LOGI(TAG, "got IP: " IPSTR, IP2STR(&event->ip_info.ip));
        xEventGroupSetBits(s_events, EVT_GOT_IP);
        emit(NET_HAL_CONNECTED, NULL, NULL);
    }
}

/* ---- net task -------------------------------------------------------- */

static void net_task(void *arg)
{
    (void)arg;

    /* NVS — provisioning_manager and WiFi need it. */
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    ESP_ERROR_CHECK(err);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_PROV_EVENT, ESP_EVENT_ANY_ID,
                                               &on_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, ESP_EVENT_ANY_ID,
                                               &on_event, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP,
                                               &on_event, NULL));

    wifi_init_config_t wifi_cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&wifi_cfg));

    /* Bring up the provisioning manager so we can ask whether creds exist.
     * If they don't we keep it running for the actual BLE flow; if they do
     * we deinit immediately and start WiFi STA ourselves. */
    wifi_prov_mgr_config_t prov_cfg = {
        .scheme               = wifi_prov_scheme_ble,
        .scheme_event_handler = WIFI_PROV_SCHEME_BLE_EVENT_HANDLER_FREE_BTDM,
    };
    ESP_ERROR_CHECK(wifi_prov_mgr_init(prov_cfg));

    bool provisioned = false;
    ESP_ERROR_CHECK(wifi_prov_mgr_is_provisioned(&provisioned));

    if (!provisioned) {
        build_service_name(s_service_name, sizeof(s_service_name));
        ESP_LOGI(TAG, "no creds; starting BLE provisioning as %s", s_service_name);
        emit(NET_HAL_PROVISIONING, s_service_name, PROV_POP);

        const char *pop = PROV_POP;
        ESP_ERROR_CHECK(wifi_prov_mgr_start_provisioning(
            WIFI_PROV_SECURITY_1, (const void *)pop,
            s_service_name, NULL));

        /* Block here until provisioning ends — manager auto-starts WiFi
         * and connects once it has good creds, so GOT_IP eventually fires. */
        wifi_prov_mgr_wait();
        /* manager has self-deinit'd in WIFI_PROV_END handler. */
    } else {
        ESP_LOGI(TAG, "stored creds present; skipping provisioning");
        wifi_prov_mgr_deinit();
        emit(NET_HAL_CONNECTING, NULL, NULL);
        ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
        ESP_ERROR_CHECK(esp_wifi_start());
    }

    /* Task is done — the event handler keeps everything live. */
    vTaskDelete(NULL);
}

/* ---- public API ------------------------------------------------------ */

esp_err_t net_hal_init(const net_hal_config_t *cfg)
{
    if (cfg) {
        s_user_cb  = cfg->event_cb;
        s_user_ctx = cfg->event_ctx;
    }
    s_events = xEventGroupCreate();
    if (!s_events) return ESP_ERR_NO_MEM;

    BaseType_t ok = xTaskCreate(net_task, "net_hal", 6144, NULL, 5, NULL);
    return ok == pdPASS ? ESP_OK : ESP_FAIL;
}

void net_hal_reset_credentials(void)
{
    ESP_LOGW(TAG, "resetting WiFi credentials and rebooting");
    /* Safe to call without manager init — uses esp_wifi_restore() under the
     * hood, which clears the wifi nvs namespace. */
    wifi_prov_mgr_reset_provisioning();
    /* Give logs a chance to flush, then restart. */
    vTaskDelay(pdMS_TO_TICKS(300));
    esp_restart();
}

bool net_hal_is_connected(void)
{
    return s_state == NET_HAL_CONNECTED;
}
