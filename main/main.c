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
#include "ui_manager.h"

static const char *TAG = "ESP32C6-TEMPLATE";

// Main application
void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C6-LCD-1.47 Template Starting...");

    // Initialize hardware components
    ESP_ERROR_CHECK(display_hal_init());
    ESP_ERROR_CHECK(led_hal_init());
    
    // Initialize UI system
    ESP_ERROR_CHECK(ui_manager_init());

    ESP_LOGI(TAG, "Template ready! All systems initialized.");

    // Run main UI loop
    ui_manager_run();
}
