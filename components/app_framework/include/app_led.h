/*
 * app_led — RGB LED control.
 *
 * Wraps the board's single WS2812 LED. All functions are thread-safe.
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/** @brief Set the LED to a steady color. */
esp_err_t app_led_set(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief Light the LED for @p ms milliseconds, then turn it off.
 *
 * Overlapping flashes re-arm the timer rather than stacking; the most recent
 * call wins.
 */
esp_err_t app_led_flash(uint8_t r, uint8_t g, uint8_t b, uint32_t ms);

/**
 * @brief Pulse the LED on/off with the given period until app_led_off() is
 *        called or another LED function changes state.
 *
 * Each half-period the LED toggles between (r,g,b) and off. A period_ms of 0
 * stops pulsing.
 */
esp_err_t app_led_pulse(uint8_t r, uint8_t g, uint8_t b, uint32_t period_ms);

/** @brief Turn off the LED and cancel any active pulse. */
esp_err_t app_led_off(void);

#ifdef __cplusplus
}
#endif
