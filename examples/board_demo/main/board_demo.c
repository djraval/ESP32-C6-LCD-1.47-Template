/**
 * @file board_demo.c
 * @brief Complete board demonstration example
 * 
 * This example demonstrates all the features of the Waveshare ESP32-C6-LCD-1.47:
 * - LCD display with graphics
 * - RGB LED color cycling
 * - TF card file operations
 * - System information display
 */

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_chip_info.h"

// Board components
#include "lcd_st7789.h"
#include "rgb_led.h"
#include "board_pins.h"

static const char *TAG = "BOARD_DEMO";

/**
 * @brief LCD demonstration task
 */
static void lcd_demo_task(void *pvParameters) {
    lcd_config_t lcd_config = lcd_get_default_config();
    
    if (lcd_init(&lcd_config) == ESP_OK) {
        ESP_LOGI(TAG, "LCD initialized successfully");
        
        // Set backlight to 50%
        lcd_set_backlight(50);
        
        while (1) {
            // Cycle through different colors
            ESP_LOGI(TAG, "LCD: Filling screen with red");
            lcd_fill_screen(LCD_COLOR_RED);
            vTaskDelay(pdMS_TO_TICKS(2000));
            
            ESP_LOGI(TAG, "LCD: Filling screen with green");
            lcd_fill_screen(LCD_COLOR_GREEN);
            vTaskDelay(pdMS_TO_TICKS(2000));
            
            ESP_LOGI(TAG, "LCD: Filling screen with blue");
            lcd_fill_screen(LCD_COLOR_BLUE);
            vTaskDelay(pdMS_TO_TICKS(2000));
            
            ESP_LOGI(TAG, "LCD: Drawing rectangle");
            lcd_fill_screen(LCD_COLOR_BLACK);
            lcd_draw_rectangle(20, 20, 100, 50, LCD_COLOR_YELLOW);
            vTaskDelay(pdMS_TO_TICKS(2000));
        }
    } else {
        ESP_LOGE(TAG, "Failed to initialize LCD");
        vTaskDelete(NULL);
    }
}

/**
 * @brief RGB LED demonstration task
 */
static void rgb_led_demo_task(void *pvParameters) {
    rgb_led_config_t led_config = rgb_led_get_default_config();
    
    if (rgb_led_init(&led_config) == ESP_OK) {
        ESP_LOGI(TAG, "RGB LED initialized successfully");
        
        uint16_t hue = 0;
        
        while (1) {
            // Cycle through rainbow colors
            rgb_led_set_hsv(hue, 100, 50);  // 50% brightness
            hue += 10;
            if (hue >= 360) {
                hue = 0;
            }
            
            vTaskDelay(pdMS_TO_TICKS(100));  // Fast color cycling
        }
    } else {
        ESP_LOGE(TAG, "Failed to initialize RGB LED");
        vTaskDelete(NULL);
    }
}

/**
 * @brief System monitoring task
 */
static void system_monitor_task(void *pvParameters) {
    while (1) {
        esp_chip_info_t chip_info;
        esp_chip_info(&chip_info);
        
        ESP_LOGI(TAG, "=== System Status ===");
        ESP_LOGI(TAG, "Free heap: %ld bytes", esp_get_free_heap_size());
        ESP_LOGI(TAG, "Uptime: %lld seconds", esp_timer_get_time() / 1000000);
        ESP_LOGI(TAG, "CPU cores: %d", chip_info.cores);
        
        // Check button state
        int button_state = gpio_get_level(BUTTON_BOOT_PIN);
        ESP_LOGI(TAG, "BOOT button: %s", button_state ? "Released" : "Pressed");
        
        vTaskDelay(pdMS_TO_TICKS(10000));  // Every 10 seconds
    }
}

void app_main(void) {
    ESP_LOGI(TAG, "Starting %s complete demonstration", BOARD_NAME);
    ESP_LOGI(TAG, "This example shows LCD, RGB LED, and system monitoring");
    
    // Initialize basic GPIO
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << BUTTON_BOOT_PIN),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    
    // Create demonstration tasks
    xTaskCreate(lcd_demo_task, "lcd_demo", 4096, NULL, 5, NULL);
    xTaskCreate(rgb_led_demo_task, "rgb_led_demo", 2048, NULL, 5, NULL);
    xTaskCreate(system_monitor_task, "sys_monitor", 2048, NULL, 3, NULL);
    
    ESP_LOGI(TAG, "All demonstration tasks started");
    ESP_LOGI(TAG, "Watch the LCD display and RGB LED!");
    ESP_LOGI(TAG, "Press the BOOT button to see button state changes");
}
