/*
 * ESP32-C6-LCD-1.47 Minimal Template
 *
 * A clean, modular starting template for the Waveshare ESP32-C6-LCD-1.47 development board.
 * Hardware initialization is abstracted into separate HAL components.
 *
 * Architecture:
 * - board_hal: Hardware abstraction layer for display, LED, and other peripherals
 * - app_ui: UI management and LVGL integration
 * - main: High-level application control and coordination
 */

#include <stdio.h>
#include "esp_log.h"
#include "display_hal.h"
#include "led_hal.h"
#include "wifi_hal.h"
#include "time_hal.h"
#include "ui_manager.h"

static const char *TAG = "ESP32C6-TEMPLATE";

// WiFi status callback
static void wifi_status_callback(wifi_status_t status, const char* ip_addr)
{
    switch (status) {
        case WIFI_STATUS_CONNECTING:
            ESP_LOGI(TAG, "WiFi connecting...");
            led_hal_set_color(255, 255, 0); // Yellow while connecting
            break;
        case WIFI_STATUS_CONNECTED:
            ESP_LOGI(TAG, "WiFi connected! IP: %s", ip_addr ? ip_addr : "Unknown");
            led_hal_set_color(0, 255, 0); // Green when connected
            // Start NTP sync after WiFi connection
            time_hal_start_sync(NULL);
            break;
        case WIFI_STATUS_FAILED:
            ESP_LOGE(TAG, "WiFi connection failed");
            led_hal_set_color(255, 0, 0); // Red on failure
            break;
        case WIFI_STATUS_DISCONNECTED:
            ESP_LOGI(TAG, "WiFi disconnected");
            led_hal_clear();
            break;
    }
}

// Main application
void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C6-LCD-1.47 Template Starting...");

    // Initialize hardware components
    ESP_ERROR_CHECK(display_hal_init());
    ESP_ERROR_CHECK(led_hal_init());

    // Initialize WiFi and time systems
    ESP_ERROR_CHECK(wifi_hal_init());
    ESP_ERROR_CHECK(time_hal_init());

    // Initialize UI system
    ESP_ERROR_CHECK(ui_manager_init());

    ESP_LOGI(TAG, "Template ready! All systems initialized.");

    // Connect to WiFi using configured credentials
    ESP_LOGI(TAG, "Connecting to WiFi...");
    wifi_hal_connect("YourWiFiSSID", "YourWiFiPassword", wifi_status_callback);

    // Run main UI loop
    ui_manager_run();
}
