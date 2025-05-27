/*
 * ESP32-C6 Basic Application
 *
 * SETUP ASSUMPTIONS:
 * - ESP32-C6FH4 (QFN32) revision v0.1 with 2MB flash
 * - Development on Windows with VS Code + ESP-IDF v5.4.1
 * - Serial port COM13 configured in VS Code settings
 * - Size optimization enabled (see sdkconfig.defaults)
 * - Basic logging at INFO level for development
 *
 * This is a minimal "Hello World" application demonstrating:
 * - Basic ESP-IDF project structure
 * - FreeRTOS task usage
 * - ESP logging system
 * - Periodic status output
 *
 * To extend this application:
 * 1. Add WiFi: Include esp_wifi.h and configure in sdkconfig.defaults
 * 2. Add Bluetooth: Include esp_bt.h and enable BT in sdkconfig.defaults
 * 3. Add sensors: Create new components in components/ directory
 * 4. Add OTA: Include esp_ota_ops.h for over-the-air updates
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

static const char *TAG = "ESP32C6_PROJECT";

void app_main(void)
{
    ESP_LOGI(TAG, "Hello World from ESP32-C6!");
    
    while (1) {
        ESP_LOGI(TAG, "ESP32-C6 is running...");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
