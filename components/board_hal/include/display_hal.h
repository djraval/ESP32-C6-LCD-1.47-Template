/*
 * Display Hardware Abstraction Layer
 * 
 * Interface for LCD display and backlight control on ESP32-C6-LCD-1.47
 */

#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_ops.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize LCD display and backlight
 * 
 * Sets up SPI bus, LCD panel (ST7789), and LEDC backlight control
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t display_hal_init(void);

/**
 * @brief Get LCD panel handle
 * 
 * @return LCD panel handle for use with LVGL or direct drawing
 */
esp_lcd_panel_handle_t display_hal_get_panel(void);

/**
 * @brief Set backlight brightness (raw PWM duty).
 *
 * @param brightness 0-255 (8-bit LEDC duty). Prefer display_hal_set_brightness().
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t display_hal_set_backlight(uint8_t brightness);

/**
 * @brief Set backlight brightness as a percentage.
 *
 * Safe to call from any task. The percent is clamped to 0-100.
 *
 * @param percent 0 (off) to 100 (full).
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t display_hal_set_brightness(uint8_t percent);

#ifdef __cplusplus
}
#endif