/*
 * Time Hardware Abstraction Layer Implementation
 * 
 * NTP time synchronization for ESP32-C6-LCD-1.47
 */

#include "time_hal.h"
#include "esp_sntp.h"
#include "esp_log.h"
#include <string.h>
#include <sys/time.h>

static const char *TAG = "TIME_HAL";

// Static variables
static time_sync_status_t s_sync_status = TIME_STATUS_NOT_SYNCED;
static time_sync_callback_t s_sync_callback = NULL;

// Default NTP servers
static const char* ntp_servers[] = {
    "pool.ntp.org",
    "time.nist.gov",
    "time.google.com"
};

static void time_sync_notification_cb(struct timeval *tv)
{
    ESP_LOGI(TAG, "Time synchronized");
    s_sync_status = TIME_STATUS_SYNCED;
    
    if (s_sync_callback) {
        s_sync_callback(s_sync_status);
    }
}

esp_err_t time_hal_init(void)
{
    ESP_LOGI(TAG, "Initializing NTP time synchronization");
    
    // Set timezone to UTC by default
    setenv("TZ", "UTC0", 1);
    tzset();
    
    ESP_LOGI(TAG, "Time HAL initialized successfully");
    return ESP_OK;
}

esp_err_t time_hal_start_sync(time_sync_callback_t callback)
{
    ESP_LOGI(TAG, "Starting NTP time synchronization");
    
    s_sync_callback = callback;
    s_sync_status = TIME_STATUS_SYNCING;
    
    if (s_sync_callback) {
        s_sync_callback(s_sync_status);
    }
    
    // Initialize SNTP
    esp_sntp_setoperatingmode(SNTP_OPMODE_POLL);
    
    // Set NTP servers
    for (int i = 0; i < sizeof(ntp_servers)/sizeof(ntp_servers[0]); i++) {
        esp_sntp_setservername(i, ntp_servers[i]);
        ESP_LOGI(TAG, "Set NTP server %d: %s", i, ntp_servers[i]);
    }
    
    // Set time sync notification callback
    sntp_set_time_sync_notification_cb(time_sync_notification_cb);
    
    // Start SNTP service
    esp_sntp_init();
    
    ESP_LOGI(TAG, "NTP synchronization started");
    return ESP_OK;
}

esp_err_t time_hal_stop_sync(void)
{
    ESP_LOGI(TAG, "Stopping NTP time synchronization");
    
    esp_sntp_stop();
    s_sync_status = TIME_STATUS_NOT_SYNCED;
    
    if (s_sync_callback) {
        s_sync_callback(s_sync_status);
    }
    
    return ESP_OK;
}

time_sync_status_t time_hal_get_sync_status(void)
{
    return s_sync_status;
}

esp_err_t time_hal_get_time_string(char* time_str, const char* format)
{
    if (!time_str || !format) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_sync_status != TIME_STATUS_SYNCED) {
        return ESP_ERR_INVALID_STATE;
    }
    
    time_t now;
    struct tm timeinfo;
    
    time(&now);
    localtime_r(&now, &timeinfo);
    
    strftime(time_str, 32, format, &timeinfo);
    return ESP_OK;
}

esp_err_t time_hal_get_time_struct(struct tm* timeinfo)
{
    if (!timeinfo) {
        return ESP_ERR_INVALID_ARG;
    }
    
    if (s_sync_status != TIME_STATUS_SYNCED) {
        return ESP_ERR_INVALID_STATE;
    }
    
    time_t now;
    time(&now);
    localtime_r(&now, timeinfo);
    
    return ESP_OK;
}

time_t time_hal_get_timestamp(void)
{
    if (s_sync_status != TIME_STATUS_SYNCED) {
        return 0;
    }
    
    time_t now;
    time(&now);
    return now;
}

esp_err_t time_hal_set_timezone(const char* timezone)
{
    if (!timezone) {
        return ESP_ERR_INVALID_ARG;
    }
    
    ESP_LOGI(TAG, "Setting timezone to: %s", timezone);
    
    setenv("TZ", timezone, 1);
    tzset();
    
    return ESP_OK;
}
