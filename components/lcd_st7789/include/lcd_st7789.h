/**
 * @file lcd_st7789.h
 * @brief ST7789 LCD driver for Waveshare ESP32-C6-LCD-1.47
 * 
 * This driver provides basic functionality for the 1.47" LCD display
 * on the Waveshare ESP32-C6-LCD-1.47 board.
 * 
 * Display specifications:
 * - Controller: ST7789
 * - Resolution: 172x320 pixels
 * - Colors: 262K (18-bit)
 * - Interface: SPI
 */

#pragma once

#include <stdint.h>
#include "esp_err.h"
#include "driver/spi_master.h"

#ifdef __cplusplus
extern "C" {
#endif

// Display dimensions
#define LCD_WIDTH   172
#define LCD_HEIGHT  320

// Basic colors (RGB565 format)
#define LCD_COLOR_BLACK     0x0000
#define LCD_COLOR_WHITE     0xFFFF
#define LCD_COLOR_RED       0xF800
#define LCD_COLOR_GREEN     0x07E0
#define LCD_COLOR_BLUE      0x001F
#define LCD_COLOR_YELLOW    0xFFE0
#define LCD_COLOR_MAGENTA   0xF81F
#define LCD_COLOR_CYAN      0x07FF

/**
 * @brief LCD configuration structure
 */
typedef struct {
    int pin_mosi;       ///< SPI MOSI pin
    int pin_sclk;       ///< SPI SCLK pin
    int pin_cs;         ///< Chip select pin
    int pin_dc;         ///< Data/Command pin
    int pin_rst;        ///< Reset pin
    int pin_bl;         ///< Backlight pin
    spi_host_device_t spi_host;  ///< SPI host to use
} lcd_config_t;

/**
 * @brief Initialize the LCD display
 * 
 * @param config LCD configuration structure
 * @return esp_err_t ESP_OK on success
 */
esp_err_t lcd_init(const lcd_config_t *config);

/**
 * @brief Set backlight brightness
 * 
 * @param brightness Brightness level (0-100)
 * @return esp_err_t ESP_OK on success
 */
esp_err_t lcd_set_backlight(uint8_t brightness);

/**
 * @brief Fill entire screen with a color
 * 
 * @param color Color in RGB565 format
 * @return esp_err_t ESP_OK on success
 */
esp_err_t lcd_fill_screen(uint16_t color);

/**
 * @brief Draw a pixel at specified coordinates
 * 
 * @param x X coordinate
 * @param y Y coordinate
 * @param color Color in RGB565 format
 * @return esp_err_t ESP_OK on success
 */
esp_err_t lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief Draw a rectangle
 * 
 * @param x X coordinate of top-left corner
 * @param y Y coordinate of top-left corner
 * @param width Rectangle width
 * @param height Rectangle height
 * @param color Color in RGB565 format
 * @return esp_err_t ESP_OK on success
 */
esp_err_t lcd_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color);

/**
 * @brief Convert RGB888 to RGB565
 * 
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return uint16_t Color in RGB565 format
 */
static inline uint16_t lcd_rgb_to_565(uint8_t r, uint8_t g, uint8_t b) {
    return ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3);
}

/**
 * @brief Get default LCD configuration for the board
 * 
 * @return lcd_config_t Default configuration
 */
lcd_config_t lcd_get_default_config(void);

#ifdef __cplusplus
}
#endif
