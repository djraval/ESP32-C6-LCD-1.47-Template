/*
 * Board Configuration for Waveshare ESP32-C6-LCD-1.47
 * 
 * Pin definitions and hardware constants for the development board.
 */

#pragma once

#include "sdkconfig.h"

#ifdef __cplusplus
extern "C" {
#endif

// LCD SPI Interface Pins
#define LCD_HOST        SPI2_HOST
#define PIN_SCLK        7
#define PIN_MOSI        6
#define PIN_LCD_DC      15
#define PIN_LCD_RST     21
#define PIN_LCD_CS      14

// LCD Display Configuration
#define LCD_H_RES       172
#define LCD_V_RES       320

// Backlight Control
#define PIN_BK_LIGHT    22

// Default backlight brightness (0-100%). Applied during display_hal_init.
// Edit via `make menuconfig` → "Display" → "Default LCD backlight brightness".
// Change at runtime with display_hal_set_brightness(percent).
#define BACKLIGHT_DEFAULT_PERCENT   CONFIG_BOARD_BACKLIGHT_PERCENT

// RGB LED
#define RGB_LED_PIN     8

#ifdef __cplusplus
}
#endif