/*
 * LED Hardware Abstraction Layer
 * 
 * Interface for RGB LED control on ESP32-C6-LCD-1.47
 */

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize RGB LED
 * 
 * Sets up WS2812 RGB LED using RMT driver
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_hal_init(void);

/**
 * @brief Set RGB LED color
 * 
 * @param red Red component (0-255)
 * @param green Green component (0-255) 
 * @param blue Blue component (0-255)
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_hal_set_color(uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Turn off RGB LED
 *
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_hal_clear(void);

/**
 * @brief Briefly flash the LED, then turn it off.
 *
 * Safe to call from any task. The first call lazily creates an internal
 * esp_timer; subsequent calls re-arm it so the most recent flash wins
 * (overlapping flashes do not stack).
 *
 * @param red          0-255
 * @param green        0-255
 * @param blue         0-255
 * @param duration_ms  how long to hold the color before clearing
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t led_hal_flash(uint8_t red, uint8_t green, uint8_t blue, uint32_t duration_ms);

#ifdef __cplusplus
}
#endif