/**
 * @file rgb_led.h
 * @brief RGB LED driver for Waveshare ESP32-C6-LCD-1.47
 * 
 * This driver provides control for the onboard WS2812-style RGB LED
 * on the Waveshare ESP32-C6-LCD-1.47 board.
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief RGB LED configuration structure
 */
typedef struct {
    int gpio_pin;           ///< GPIO pin connected to LED
    int led_count;          ///< Number of LEDs in chain
    int rmt_channel;        ///< RMT channel to use
} rgb_led_config_t;

/**
 * @brief Initialize the RGB LED
 * 
 * @param config LED configuration structure
 * @return esp_err_t ESP_OK on success
 */
esp_err_t rgb_led_init(const rgb_led_config_t *config);

/**
 * @brief Set RGB LED color
 * 
 * @param red Red component (0-255)
 * @param green Green component (0-255)
 * @param blue Blue component (0-255)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t rgb_led_set_color(uint8_t red, uint8_t green, uint8_t blue);

/**
 * @brief Set RGB LED color with HSV
 * 
 * @param hue Hue (0-360)
 * @param saturation Saturation (0-100)
 * @param value Value/Brightness (0-100)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t rgb_led_set_hsv(uint16_t hue, uint8_t saturation, uint8_t value);

/**
 * @brief Turn off the RGB LED
 * 
 * @return esp_err_t ESP_OK on success
 */
esp_err_t rgb_led_off(void);

/**
 * @brief Get default RGB LED configuration for the board
 * 
 * @return rgb_led_config_t Default configuration
 */
rgb_led_config_t rgb_led_get_default_config(void);

// Predefined colors
#define RGB_LED_COLOR_RED       255, 0, 0
#define RGB_LED_COLOR_GREEN     0, 255, 0
#define RGB_LED_COLOR_BLUE      0, 0, 255
#define RGB_LED_COLOR_WHITE     255, 255, 255
#define RGB_LED_COLOR_YELLOW    255, 255, 0
#define RGB_LED_COLOR_MAGENTA   255, 0, 255
#define RGB_LED_COLOR_CYAN      0, 255, 255
#define RGB_LED_COLOR_ORANGE    255, 165, 0
#define RGB_LED_COLOR_PURPLE    128, 0, 128
#define RGB_LED_COLOR_OFF       0, 0, 0

#ifdef __cplusplus
}
#endif
