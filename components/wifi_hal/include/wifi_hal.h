/*
 * WiFi Hardware Abstraction Layer
 * 
 * Interface for WiFi connection management on ESP32-C6-LCD-1.47
 */

#pragma once

#include "esp_err.h"
#include "esp_wifi_types.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief WiFi connection status
 */
typedef enum {
    WIFI_STATUS_DISCONNECTED = 0,
    WIFI_STATUS_CONNECTING,
    WIFI_STATUS_CONNECTED,
    WIFI_STATUS_FAILED
} wifi_status_t;

/**
 * @brief WiFi connection callback function type
 * 
 * @param status Current WiFi status
 * @param ip_addr IP address string (NULL if not connected)
 */
typedef void (*wifi_status_callback_t)(wifi_status_t status, const char* ip_addr);

/**
 * @brief Initialize WiFi subsystem
 * 
 * Sets up WiFi in station mode and prepares for connection
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t wifi_hal_init(void);

/**
 * @brief Connect to WiFi network
 * 
 * @param ssid WiFi network SSID
 * @param password WiFi network password
 * @param callback Optional callback for status updates (can be NULL)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t wifi_hal_connect(const char* ssid, const char* password, wifi_status_callback_t callback);

/**
 * @brief Disconnect from WiFi network
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t wifi_hal_disconnect(void);

/**
 * @brief Get current WiFi connection status
 * 
 * @return Current WiFi status
 */
wifi_status_t wifi_hal_get_status(void);

/**
 * @brief Get current IP address
 * 
 * @param ip_str Buffer to store IP address string (minimum 16 bytes)
 * @return ESP_OK if connected and IP retrieved, ESP_ERR_INVALID_STATE if not connected
 */
esp_err_t wifi_hal_get_ip_address(char* ip_str);

/**
 * @brief Get WiFi signal strength (RSSI)
 * 
 * @return RSSI value in dBm, or 0 if not connected
 */
int8_t wifi_hal_get_rssi(void);

#ifdef __cplusplus
}
#endif
