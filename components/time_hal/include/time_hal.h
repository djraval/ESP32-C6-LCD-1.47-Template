/*
 * Time Hardware Abstraction Layer
 * 
 * Interface for NTP time synchronization on ESP32-C6-LCD-1.47
 */

#pragma once

#include "esp_err.h"
#include <time.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Time synchronization status
 */
typedef enum {
    TIME_STATUS_NOT_SYNCED = 0,
    TIME_STATUS_SYNCING,
    TIME_STATUS_SYNCED
} time_sync_status_t;

/**
 * @brief Time sync callback function type
 * 
 * @param status Current time sync status
 */
typedef void (*time_sync_callback_t)(time_sync_status_t status);

/**
 * @brief Initialize NTP time synchronization
 * 
 * Sets up SNTP client and configures NTP servers
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t time_hal_init(void);

/**
 * @brief Start NTP time synchronization
 * 
 * @param callback Optional callback for sync status updates (can be NULL)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t time_hal_start_sync(time_sync_callback_t callback);

/**
 * @brief Stop NTP time synchronization
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t time_hal_stop_sync(void);

/**
 * @brief Get current time synchronization status
 * 
 * @return Current time sync status
 */
time_sync_status_t time_hal_get_sync_status(void);

/**
 * @brief Get current time as formatted string
 * 
 * @param time_str Buffer to store time string (minimum 32 bytes)
 * @param format Time format string (strftime format)
 * @return ESP_OK if time is available, ESP_ERR_INVALID_STATE if not synced
 */
esp_err_t time_hal_get_time_string(char* time_str, const char* format);

/**
 * @brief Get current time as struct tm
 * 
 * @param timeinfo Pointer to tm structure to fill
 * @return ESP_OK if time is available, ESP_ERR_INVALID_STATE if not synced
 */
esp_err_t time_hal_get_time_struct(struct tm* timeinfo);

/**
 * @brief Get current Unix timestamp
 * 
 * @return Unix timestamp, or 0 if time not synced
 */
time_t time_hal_get_timestamp(void);

/**
 * @brief Set timezone
 * 
 * @param timezone Timezone string (e.g., "EST5EDT,M3.2.0/2,M11.1.0")
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t time_hal_set_timezone(const char* timezone);

#ifdef __cplusplus
}
#endif
