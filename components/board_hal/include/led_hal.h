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

#ifdef __cplusplus
}
#endif