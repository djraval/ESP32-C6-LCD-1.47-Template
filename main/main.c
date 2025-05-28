/*
 * ESP32-C6 Hello World with LCD Display
 *
 * Simple hello world application for the Waveshare ESP32-C6-LCD-1.47 board.
 * Demonstrates LCD usage with bezel compensation for curved display edges.
 *
 * Features:
 * - System information display in console
 * - Visual feedback on 1.47" LCD display
 * - Safe area rendering avoiding curved bezel edges
 * - Text rendering with 8x8 bitmap font
 * - Color cycling every 5 seconds with dynamic text
 *
 * Board: Waveshare ESP32-C6-LCD-1.47
 * - ESP32-C6FH4 (QFN32) with 4MB flash
 * - 1.47" LCD display (ST7789, 172x320) with bezel compensation
 * - RGB LED (WS2812-style)
 * - TF card slot for external storage
 *
 * To extend this application:
 * 1. ✅ Text rendering implemented with full font support
 * 2. ✅ RGB LED implemented with WS2812 RMT driver
 * 3. Add WiFi/Bluetooth connectivity
 * 4. See examples/board_demo/ for advanced features
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"
#include "esp_chip_info.h"
#include "esp_flash.h"
#include "esp_timer.h"
#include "lcd_st7789.h"
#include "rgb_led.h"
#include "board_pins.h"

static const char *TAG = "ESP32C6_HELLO";

// Global hardware state
static bool lcd_ready = false;
static bool rgb_led_ready = false;

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
 * @brief Initialize LCD display
 */
static void init_lcd(void)
{
    ESP_LOGI(TAG, "Initializing LCD display...");

    lcd_config_t lcd_config = lcd_get_default_config();

    if (lcd_init(&lcd_config) == ESP_OK) {
        ESP_LOGI(TAG, "LCD initialized successfully");

        // Set backlight to 75%
        lcd_set_backlight(75);

        // Clear screen and demonstrate text rendering
        lcd_fill_screen(LCD_COLOR_BLACK);

        // Draw text demonstration with smart layout
        uint16_t current_y = 0;
        lcd_text_bounds_t bounds;

        // Title
        lcd_draw_safe_string(0, current_y, "ESP32-C6 Text Demo", LCD_COLOR_WHITE, LCD_COLOR_BLACK);
        current_y += 15; // Fixed spacing for single line

        // Demonstrate text wrapping with bounds calculation
        lcd_draw_safe_text_wrapped_ex(0, current_y, "This is a long line that will automatically wrap to the next line when it reaches the edge of the safe area!",
                                     LCD_COLOR_YELLOW, LCD_COLOR_BLACK, 1, &bounds);
        ESP_LOGI(TAG, "Wrapped text: %d lines, %dx%d pixels", bounds.lines, bounds.width, bounds.height);
        current_y += bounds.height + 5; // Add 5 pixels spacing

        // Demonstrate explicit line breaks with bounds
        lcd_draw_safe_text_wrapped_ex(0, current_y, "Line 1\nLine 2\nLine 3", LCD_COLOR_GREEN, LCD_COLOR_BLACK, 1, &bounds);
        ESP_LOGI(TAG, "Multi-line text: %d lines, %dx%d pixels", bounds.lines, bounds.width, bounds.height);
        current_y += bounds.height + 5; // Add 5 pixels spacing

        // Demonstrate printf-style formatting with bounds
        lcd_printf_safe_ex(0, current_y, LCD_COLOR_CYAN, LCD_COLOR_BLACK, 1, &bounds,
                          "Heap: %ld bytes\nTime: %d ms\nY-pos: %d",
                          esp_get_free_heap_size(), (int)(esp_timer_get_time() / 1000), current_y);
        ESP_LOGI(TAG, "Printf text: %d lines, %dx%d pixels", bounds.lines, bounds.width, bounds.height);

        ESP_LOGI(TAG, "Text rendering demo: multiple sizes and cases");
        ESP_LOGI(TAG, "Safe area: %dx%d pixels, avoiding curved bezel edges",
                 LCD_SAFE_WIDTH, LCD_SAFE_HEIGHT);

        vTaskDelay(pdMS_TO_TICKS(4000));

        lcd_ready = true;
        ESP_LOGI(TAG, "LCD ready for display");
    } else {
        ESP_LOGE(TAG, "Failed to initialize LCD");
        lcd_ready = false;
    }
}

/**
 * @brief Initialize RGB LED
 */
static void init_rgb_led(void)
{
    ESP_LOGI(TAG, "Initializing RGB LED...");

    rgb_led_config_t rgb_config = rgb_led_get_default_config();

    if (rgb_led_init(&rgb_config) == ESP_OK) {
        ESP_LOGI(TAG, "RGB LED initialized successfully");

        // Run comprehensive test sequence
        rgb_led_test();

        rgb_led_ready = true;
        ESP_LOGI(TAG, "RGB LED ready for use");
    } else {
        ESP_LOGE(TAG, "Failed to initialize RGB LED");
        rgb_led_ready = false;
    }
}

/**
 * @brief Visual feedback task using LCD screen
 */
static void hello_task(void *pvParameters)
{
    int counter = 0;
    uint16_t lcd_colors[] = {
        LCD_COLOR_RED,
        LCD_COLOR_GREEN,
        LCD_COLOR_BLUE,
        LCD_COLOR_YELLOW,
        LCD_COLOR_MAGENTA,
        LCD_COLOR_CYAN,
        LCD_COLOR_WHITE
    };

    // RGB LED colors (R, G, B values)
    struct {
        uint8_t r, g, b;
    } rgb_colors[] = {
        {255, 0, 0},    // Red
        {0, 255, 0},    // Green
        {0, 0, 255},    // Blue
        {255, 255, 0},  // Yellow
        {255, 0, 255},  // Magenta
        {0, 255, 255},  // Cyan
        {255, 255, 255} // White
    };

    int color_count = sizeof(lcd_colors) / sizeof(lcd_colors[0]);

    while (1) {
        ESP_LOGI(TAG, "Hello World! Counter: %d", counter);
        ESP_LOGI(TAG, "Free heap: %ld bytes", esp_get_free_heap_size());

        // Visual feedback on LCD screen and RGB LED
        if (lcd_ready || rgb_led_ready) {
            int color_index = counter % color_count;
            uint16_t current_lcd_color = lcd_colors[color_index];

            // Set RGB LED color
            if (rgb_led_ready) {
                rgb_led_set_color(rgb_colors[color_index].r,
                                 rgb_colors[color_index].g,
                                 rgb_colors[color_index].b);
                ESP_LOGI(TAG, "RGB LED: Set to color %d (R=%d, G=%d, B=%d)",
                        color_index, rgb_colors[color_index].r,
                        rgb_colors[color_index].g, rgb_colors[color_index].b);
            }

            // Update LCD display
            if (lcd_ready) {
                // Fill safe area with color (avoiding curved bezel)
                lcd_fill_screen(LCD_COLOR_BLACK);
                lcd_fill_safe_area(current_lcd_color);

                // Draw text on colored background with different sizes
                // Alternate between different font sizes based on counter
                if (counter % 3 == 0) {
                    // Small text with wrapping
                    lcd_printf_safe(5, 5, LCD_COLOR_BLACK, current_lcd_color, 1,
                                   "Count: %d\nSmall Font\nHeap: %ld\nRGB: %s",
                                   counter, esp_get_free_heap_size(),
                                   rgb_led_ready ? "ON" : "OFF");
                } else if (counter % 3 == 1) {
                    // Medium text (2x scale)
                    lcd_printf_safe(5, 5, LCD_COLOR_BLACK, current_lcd_color, 2,
                                   "Count: %d\nMedium\nRGB: %s", counter,
                                   rgb_led_ready ? "ON" : "OFF");
                } else {
                    // Large text (3x scale)
                    lcd_printf_safe(5, 5, LCD_COLOR_BLACK, current_lcd_color, 3,
                                   "Count: %d\nBig!", counter);
                }

                ESP_LOGI(TAG, "LCD: Safe area updated with color 0x%04X", current_lcd_color);
            }
        }

        counter++;

        // Wait 5 seconds (slower updates = less noticeable tearing)
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C6 Hello World with LCD Display starting...");

    // Display basic system information
    print_system_info();

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Initializing hardware...");

    // Initialize LCD display
    init_lcd();

    // Initialize RGB LED
    init_rgb_led();

    ESP_LOGI(TAG, "");
    ESP_LOGI(TAG, "Hardware initialized! Features available:");
    ESP_LOGI(TAG, "✅ LCD Display: %s", lcd_ready ? "Ready" : "Failed");
    ESP_LOGI(TAG, "✅ RGB LED: %s", rgb_led_ready ? "Ready" : "Failed");
    ESP_LOGI(TAG, "- WiFi/Bluetooth: Configure in sdkconfig.defaults");
    ESP_LOGI(TAG, "- Advanced features: See examples/board_demo/");
    ESP_LOGI(TAG, "");

    // Create and start the hello task
    xTaskCreate(hello_task, "hello_task", 4096, NULL, 5, NULL);

    if (lcd_ready || rgb_led_ready) {
        ESP_LOGI(TAG, "Visual feedback started!");
        if (lcd_ready) {
            ESP_LOGI(TAG, "- LCD screen will cycle through colors every 5 seconds");
        }
        if (rgb_led_ready) {
            ESP_LOGI(TAG, "- RGB LED will sync with LCD colors");
        }
        ESP_LOGI(TAG, "Console logs will continue for debugging.");
    } else {
        ESP_LOGI(TAG, "Hardware initialization failed. Check console logs for debugging.");
    }
}
