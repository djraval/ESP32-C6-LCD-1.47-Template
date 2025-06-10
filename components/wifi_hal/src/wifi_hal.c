/*
 * WiFi Hardware Abstraction Layer Implementation
 * 
 * WiFi connection management for ESP32-C6-LCD-1.47
 */

#include "wifi_hal.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"
#include <string.h>

static const char *TAG = "WIFI_HAL";

// Event group bits
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Static variables
static EventGroupHandle_t s_wifi_event_group;
static wifi_status_t s_wifi_status = WIFI_STATUS_DISCONNECTED;
static wifi_status_callback_t s_status_callback = NULL;
static int s_retry_num = 0;
static char s_ip_address[16] = {0};

static void event_handler(void* arg, esp_event_base_t event_base,
                         int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
        s_wifi_status = WIFI_STATUS_CONNECTING;
        if (s_status_callback) {
            s_status_callback(s_wifi_status, NULL);
        }
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        if (s_retry_num < 5) {
            esp_wifi_connect();
            s_retry_num++;
            ESP_LOGI(TAG, "Retry to connect to the AP");
            s_wifi_status = WIFI_STATUS_CONNECTING;
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
            s_wifi_status = WIFI_STATUS_FAILED;
            ESP_LOGI(TAG, "Connect to the AP failed");
        }
        if (s_status_callback) {
            s_status_callback(s_wifi_status, NULL);
        }
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        ip_event_got_ip_t* event = (ip_event_got_ip_t*) event_data;
        snprintf(s_ip_address, sizeof(s_ip_address), IPSTR, IP2STR(&event->ip_info.ip));
        ESP_LOGI(TAG, "Got IP: %s", s_ip_address);
        s_retry_num = 0;
        s_wifi_status = WIFI_STATUS_CONNECTED;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
        if (s_status_callback) {
            s_status_callback(s_wifi_status, s_ip_address);
        }
    }
}

esp_err_t wifi_hal_init(void)
{
    ESP_LOGI(TAG, "Initializing WiFi");
    
    s_wifi_event_group = xEventGroupCreate();
    
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    
    ESP_LOGI(TAG, "WiFi initialized successfully");
    return ESP_OK;
}

esp_err_t wifi_hal_connect(const char* ssid, const char* password, wifi_status_callback_t callback)
{
    if (!ssid) {
        ESP_LOGE(TAG, "SSID cannot be NULL");
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Connecting to WiFi SSID: %s", ssid);
    
    s_status_callback = callback;
    s_retry_num = 0;
    
    wifi_config_t wifi_config = {0};
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid) - 1);
    if (password) {
        strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password) - 1);
    }
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    ESP_ERROR_CHECK(esp_wifi_start());

    ESP_LOGI(TAG, "WiFi connection initiated");
    return ESP_OK;
}

esp_err_t wifi_hal_disconnect(void)
{
    ESP_LOGI(TAG, "Disconnecting from WiFi");
    
    esp_err_t ret = esp_wifi_disconnect();
    if (ret == ESP_OK) {
        s_wifi_status = WIFI_STATUS_DISCONNECTED;
        memset(s_ip_address, 0, sizeof(s_ip_address));
        if (s_status_callback) {
            s_status_callback(s_wifi_status, NULL);
        }
    }
    
    return ret;
}

wifi_status_t wifi_hal_get_status(void)
{
    return s_wifi_status;
}

esp_err_t wifi_hal_get_ip_address(char* ip_str)
{
    if (!ip_str) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_wifi_status != WIFI_STATUS_CONNECTED || strlen(s_ip_address) == 0) {
        return ESP_ERR_INVALID_STATE;
    }
    
    strcpy(ip_str, s_ip_address);
    return ESP_OK;
}

int8_t wifi_hal_get_rssi(void)
{
    if (s_wifi_status != WIFI_STATUS_CONNECTED) {
        return 0;
    }
    
    wifi_ap_record_t ap_info;
    esp_err_t ret = esp_wifi_sta_get_ap_info(&ap_info);
    if (ret == ESP_OK) {
        return ap_info.rssi;
    }
    
    return 0;
}
