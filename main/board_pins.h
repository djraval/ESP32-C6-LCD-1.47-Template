/**
 * @file board_pins.h
 * @brief Pin definitions for Waveshare ESP32-C6-LCD-1.47 development board
 * 
 * This file contains all GPIO pin definitions and hardware configuration
 * for the Waveshare ESP32-C6-LCD-1.47 board.
 * 
 * Board Features:
 * - 1.47" LCD Display (ST7789, 172x320)
 * - RGB LED (WS2812-style)
 * - TF Card slot
 * - USB Type-C interface
 * - BOOT and RESET buttons
 */

#pragma once

#ifdef __cplusplus
extern "C" {
#endif

// =============================================================================
// LCD Display (ST7789) - SPI Interface
// =============================================================================
#define LCD_SPI_HOST        SPI2_HOST
#define LCD_SPI_FREQ_HZ     (40 * 1000 * 1000)  // 40MHz

#define LCD_PIN_MOSI        6   // SPI MOSI (shared with SD card)
#define LCD_PIN_SCLK        7   // SPI SCLK (shared with SD card)
#define LCD_PIN_CS          14  // Chip Select
#define LCD_PIN_DC          15  // Data/Command select
#define LCD_PIN_RST         21  // Reset
#define LCD_PIN_BL          22  // Backlight control (PWM)

// LCD Display specifications
#define LCD_WIDTH           172
#define LCD_HEIGHT          320
#define LCD_ROTATION        0   // 0=portrait, 1=landscape

// =============================================================================
// RGB LED (WS2812-style)
// =============================================================================
#define RGB_LED_PIN         8
#define RGB_LED_COUNT       1   // Single RGB LED
#define RGB_LED_RMT_CHANNEL 0   // RMT channel number

// =============================================================================
// TF Card (MicroSD) - SPI Interface (shared with LCD)
// =============================================================================
#define SD_SPI_HOST         SPI2_HOST  // Same as LCD
#define SD_SPI_FREQ_HZ      (20 * 1000 * 1000)  // 20MHz for SD card

#define SD_PIN_MISO         5   // SPI MISO (SD card only)
#define SD_PIN_MOSI         6   // SPI MOSI (shared with LCD)
#define SD_PIN_SCLK         7   // SPI SCLK (shared with LCD)
#define SD_PIN_CS           4   // Chip Select

// SD card mount point
#define SD_MOUNT_POINT      "/sdcard"

// =============================================================================
// Built-in Buttons
// =============================================================================
#define BUTTON_BOOT_PIN     0   // Built-in BOOT button
#define BUTTON_ACTIVE_LEVEL 0   // Active low

// =============================================================================
// Available GPIO for User Applications
// =============================================================================
// These pins are available for connecting external sensors, actuators, etc.

// Digital I/O pins (can be used for any digital purpose)
#define USER_GPIO_1         1
#define USER_GPIO_2         2
#define USER_GPIO_3         3
#define USER_GPIO_9         9
#define USER_GPIO_10        10
#define USER_GPIO_11        11
#define USER_GPIO_18        18
#define USER_GPIO_19        19
#define USER_GPIO_20        20
#define USER_GPIO_23        23

// Analog input pins (ADC capable)
#define ADC_PIN_1           1   // ADC1_CH0
#define ADC_PIN_2           2   // ADC1_CH1
#define ADC_PIN_3           3   // ADC1_CH2

// Recommended I2C pins (can be changed if needed)
#define I2C_SDA_PIN         18
#define I2C_SCL_PIN         19
#define I2C_FREQ_HZ         100000  // 100kHz

// Recommended additional UART pins
#define UART1_TX_PIN        16
#define UART1_RX_PIN        17

// Note: UART2 pins (20/21) conflict with LCD_RST, so use UART1 for external devices

// =============================================================================
// Power and System
// =============================================================================
#define BOARD_VCC_3V3       3300    // 3.3V system voltage (mV)
#define BOARD_VCC_5V        5000    // 5V USB input voltage (mV)

// =============================================================================
// Hardware Configuration Helpers
// =============================================================================

/**
 * @brief Check if a GPIO pin is available for user applications
 * @param gpio_num GPIO number to check
 * @return true if available, false if used by board peripherals
 */
static inline bool is_gpio_available(int gpio_num) {
    // Pins used by board peripherals
    switch (gpio_num) {
        case LCD_PIN_MOSI:
        case LCD_PIN_SCLK:
        case LCD_PIN_CS:
        case LCD_PIN_DC:
        case LCD_PIN_RST:
        case LCD_PIN_BL:
        case RGB_LED_PIN:
        case SD_PIN_MISO:
        case SD_PIN_CS:
        case BUTTON_BOOT_PIN:
            return false;
        default:
            return true;
    }
}

/**
 * @brief Get a human-readable description of a GPIO pin's board function
 * @param gpio_num GPIO number
 * @return String description of the pin's function
 */
static inline const char* get_gpio_description(int gpio_num) {
    switch (gpio_num) {
        case LCD_PIN_MOSI:  return "LCD/SD MOSI";
        case LCD_PIN_SCLK:  return "LCD/SD SCLK";
        case LCD_PIN_CS:    return "LCD CS";
        case LCD_PIN_DC:    return "LCD DC";
        case LCD_PIN_RST:   return "LCD RST";
        case LCD_PIN_BL:    return "LCD Backlight";
        case RGB_LED_PIN:   return "RGB LED";
        case SD_PIN_MISO:   return "SD MISO";
        case SD_PIN_CS:     return "SD CS";
        case BUTTON_BOOT_PIN: return "BOOT Button";
        default:            return "Available";
    }
}

// =============================================================================
// Board Information
// =============================================================================
#define BOARD_NAME          "Waveshare ESP32-C6-LCD-1.47"
#define BOARD_VERSION       "1.0"
#define BOARD_URL           "https://www.waveshare.com/wiki/ESP32-C6-LCD-1.47"

#ifdef __cplusplus
}
#endif
