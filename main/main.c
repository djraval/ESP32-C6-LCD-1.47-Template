/*
 * ESP32-C6 Hello World
 *
 * Simple hello world application for the Waveshare ESP32-C6-LCD-1.47 board.
 * This is a minimal starting point for your ESP32-C6 projects.
 *
 * Board: Waveshare ESP32-C6-LCD-1.47
 * - ESP32-C6FH4 (QFN32) with 4MB flash
 * - 1.47" LCD display (ST7789, 172x320)
 * - RGB LED (WS2812-style)
 * - TF card slot for external storage
 *
 * To extend this application:
 * 1. Add LCD driver: Include "lcd_st7789.h" and configure SPI
 * 2. Add RGB LED: Include "rgb_led.h" and configure RMT
 * 3. Add WiFi: Include esp_wifi.h and configure in sdkconfig.defaults
 * 4. Add Bluetooth: Include esp_bt.h and enable BT in sdkconfig.defaults
 * 5. See examples/board_demo/ for advanced features
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"

static const char *TAG = "ESP32C6_HELLO";

/**
 * @brief Display basic system information
 */
static void print_system_info(void)
{
    esp_chip_info_t chip_info;
    uint32_t flash_size;

    esp_chip_info(&chip_info);
    esp_flash_get_size(NULL, &flash_size);

    ESP_LOGI(TAG, "=== Waveshare ESP32-C6-LCD-1.47 ===");
    ESP_LOGI(TAG, "ESP32-C6 chip with %d CPU core(s), WiFi%s%s",
             chip_info.cores,
             (chip_info.features & CHIP_FEATURE_BT) ? "/BT" : "",
             (chip_info.features & CHIP_FEATURE_BLE) ? "/BLE" : "");
    ESP_LOGI(TAG, "Silicon revision: %d", chip_info.revision);
    ESP_LOGI(TAG, "Flash size: %ld MB", flash_size / (1024 * 1024));
    ESP_LOGI(TAG, "Free heap: %ld bytes", esp_get_free_heap_size());
}

/**
 * @brief Simple counter task
 */
static void hello_task(void *pvParameters)
{
    int counter = 0;

    while (1) {
        ESP_LOGI(TAG, "Hello World! Counter: %d", counter++);
        ESP_LOGI(TAG, "Free heap: %ld bytes", esp_get_free_heap_size());

        // Wait 5 seconds
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C6 Hello World starting...");

    // Display basic system information
    print_system_info();

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Template ready! Next steps:");
    ESP_LOGI(TAG, "1. Add LCD driver: Include lcd_st7789.h");
    ESP_LOGI(TAG, "2. Add RGB LED: Include rgb_led.h");
    ESP_LOGI(TAG, "3. Add WiFi/Bluetooth connectivity");
    ESP_LOGI(TAG, "4. See examples/board_demo/ for advanced features");
    ESP_LOGI(TAG, "");

    // Create and start the hello task
    xTaskCreate(hello_task, "hello_task", 4096, NULL, 5, NULL);

    ESP_LOGI(TAG, "Hello task started. Check the output every 5 seconds!");
}
