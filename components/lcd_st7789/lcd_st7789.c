/**
 * @file lcd_st7789.c
 * @brief ST7789 LCD driver implementation
 */

#include "lcd_st7789.h"
#include "esp_log.h"
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>
// Pin definitions for ESP32-C6-LCD-1.47
#define LCD_PIN_MOSI    6
#define LCD_PIN_SCLK    7
#define LCD_PIN_CS      14
#define LCD_PIN_DC      15
#define LCD_PIN_RST     21
#define LCD_PIN_BL      22
#define LCD_SPI_HOST    SPI2_HOST
#define LCD_SPI_FREQ_HZ 80000000  // 80MHz

// Safe area definitions (avoiding curved bezel)
#define LCD_SAFE_X_START 6
#define LCD_SAFE_Y_START 8
#define LCD_SAFE_WIDTH   160
#define LCD_SAFE_HEIGHT  304

// Simple 8x8 bitmap font (ASCII 32-126)
// Each character is 8 bytes, each byte represents a row
static const uint8_t font_8x8[][8] = {
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // Space (32)
    {0x18, 0x3C, 0x3C, 0x18, 0x18, 0x00, 0x18, 0x00}, // ! (33)
    {0x36, 0x36, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}, // " (34)
    {0x36, 0x36, 0x7F, 0x36, 0x7F, 0x36, 0x36, 0x00}, // # (35)
    {0x0C, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x0C, 0x00}, // $ (36)
    {0x00, 0x63, 0x33, 0x18, 0x0C, 0x66, 0x63, 0x00}, // % (37)
    {0x1C, 0x36, 0x1C, 0x6E, 0x3B, 0x33, 0x6E, 0x00}, // & (38)
    {0x06, 0x06, 0x03, 0x00, 0x00, 0x00, 0x00, 0x00}, // ' (39)
    {0x18, 0x0C, 0x06, 0x06, 0x06, 0x0C, 0x18, 0x00}, // ( (40)
    {0x06, 0x0C, 0x18, 0x18, 0x18, 0x0C, 0x06, 0x00}, // ) (41)
    {0x00, 0x66, 0x3C, 0xFF, 0x3C, 0x66, 0x00, 0x00}, // * (42)
    {0x00, 0x0C, 0x0C, 0x3F, 0x0C, 0x0C, 0x00, 0x00}, // + (43)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x06, 0x00}, // , (44)
    {0x00, 0x00, 0x00, 0x3F, 0x00, 0x00, 0x00, 0x00}, // - (45)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // . (46)
    {0x60, 0x30, 0x18, 0x0C, 0x06, 0x03, 0x01, 0x00}, // / (47)
    {0x3E, 0x63, 0x73, 0x7B, 0x6F, 0x67, 0x3E, 0x00}, // 0 (48)
    {0x0C, 0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x3F, 0x00}, // 1 (49)
    {0x1E, 0x33, 0x30, 0x1C, 0x06, 0x33, 0x3F, 0x00}, // 2 (50)
    {0x1E, 0x33, 0x30, 0x1C, 0x30, 0x33, 0x1E, 0x00}, // 3 (51)
    {0x38, 0x3C, 0x36, 0x33, 0x7F, 0x30, 0x78, 0x00}, // 4 (52)
    {0x3F, 0x03, 0x1F, 0x30, 0x30, 0x33, 0x1E, 0x00}, // 5 (53)
    {0x1C, 0x06, 0x03, 0x1F, 0x33, 0x33, 0x1E, 0x00}, // 6 (54)
    {0x3F, 0x33, 0x30, 0x18, 0x0C, 0x0C, 0x0C, 0x00}, // 7 (55)
    {0x1E, 0x33, 0x33, 0x1E, 0x33, 0x33, 0x1E, 0x00}, // 8 (56)
    {0x1E, 0x33, 0x33, 0x3E, 0x30, 0x18, 0x0E, 0x00}, // 9 (57)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x0C, 0x00}, // : (58)
    {0x00, 0x0C, 0x0C, 0x00, 0x00, 0x0C, 0x06, 0x00}, // ; (59)
    {0x18, 0x0C, 0x06, 0x03, 0x06, 0x0C, 0x18, 0x00}, // < (60)
    {0x00, 0x00, 0x3F, 0x00, 0x00, 0x3F, 0x00, 0x00}, // = (61)
    {0x06, 0x0C, 0x18, 0x30, 0x18, 0x0C, 0x06, 0x00}, // > (62)
    {0x1E, 0x33, 0x30, 0x18, 0x0C, 0x00, 0x0C, 0x00}, // ? (63)
    {0x3E, 0x63, 0x7B, 0x7B, 0x7B, 0x03, 0x1E, 0x00}, // @ (64)
    {0x0C, 0x1E, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x00}, // A (65)
    {0x3F, 0x66, 0x66, 0x3E, 0x66, 0x66, 0x3F, 0x00}, // B (66)
    {0x3C, 0x66, 0x03, 0x03, 0x03, 0x66, 0x3C, 0x00}, // C (67)
    {0x1F, 0x36, 0x66, 0x66, 0x66, 0x36, 0x1F, 0x00}, // D (68)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x46, 0x7F, 0x00}, // E (69)
    {0x7F, 0x46, 0x16, 0x1E, 0x16, 0x06, 0x0F, 0x00}, // F (70)
    {0x3C, 0x66, 0x03, 0x03, 0x73, 0x66, 0x7C, 0x00}, // G (71)
    {0x33, 0x33, 0x33, 0x3F, 0x33, 0x33, 0x33, 0x00}, // H (72)
    {0x1E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // I (73)
    {0x78, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E, 0x00}, // J (74)
    {0x67, 0x66, 0x36, 0x1E, 0x36, 0x66, 0x67, 0x00}, // K (75)
    {0x0F, 0x06, 0x06, 0x06, 0x46, 0x66, 0x7F, 0x00}, // L (76)
    {0x63, 0x77, 0x7F, 0x7F, 0x6B, 0x63, 0x63, 0x00}, // M (77)
    {0x63, 0x67, 0x6F, 0x7B, 0x73, 0x63, 0x63, 0x00}, // N (78)
    {0x1C, 0x36, 0x63, 0x63, 0x63, 0x36, 0x1C, 0x00}, // O (79)
    {0x3F, 0x66, 0x66, 0x3E, 0x06, 0x06, 0x0F, 0x00}, // P (80)
    {0x1E, 0x33, 0x33, 0x33, 0x3B, 0x1E, 0x38, 0x00}, // Q (81)
    {0x3F, 0x66, 0x66, 0x3E, 0x36, 0x66, 0x67, 0x00}, // R (82)
    {0x1E, 0x33, 0x07, 0x0E, 0x38, 0x33, 0x1E, 0x00}, // S (83)
    {0x3F, 0x2D, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // T (84)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x33, 0x3F, 0x00}, // U (85)
    {0x33, 0x33, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // V (86)
    {0x63, 0x63, 0x63, 0x6B, 0x7F, 0x77, 0x63, 0x00}, // W (87)
    {0x63, 0x63, 0x36, 0x1C, 0x1C, 0x36, 0x63, 0x00}, // X (88)
    {0x33, 0x33, 0x33, 0x1E, 0x0C, 0x0C, 0x1E, 0x00}, // Y (89)
    {0x7F, 0x63, 0x31, 0x18, 0x4C, 0x66, 0x7F, 0x00}, // Z (90)
    {0x1C, 0x36, 0x30, 0x30, 0x30, 0x36, 0x1C, 0x00}, // [ (91)
    {0x01, 0x03, 0x06, 0x0C, 0x18, 0x30, 0x60, 0x00}, // \ (92)
    {0x0E, 0x1B, 0x03, 0x03, 0x03, 0x1B, 0x0E, 0x00}, // ] (93)
    {0x08, 0x1C, 0x36, 0x63, 0x00, 0x00, 0x00, 0x00}, // ^ (94)
    {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xFF}, // _ (95)
    {0x0C, 0x0C, 0x18, 0x00, 0x00, 0x00, 0x00, 0x00}, // ` (96)
    {0x00, 0x00, 0x1E, 0x30, 0x3E, 0x33, 0x6E, 0x00}, // a (97)
    {0x07, 0x06, 0x06, 0x3E, 0x66, 0x66, 0x3B, 0x00}, // b (98)
    {0x00, 0x00, 0x1E, 0x33, 0x03, 0x33, 0x1E, 0x00}, // c (99)
    {0x38, 0x30, 0x30, 0x3e, 0x33, 0x33, 0x6E, 0x00}, // d (100)
    {0x00, 0x00, 0x1E, 0x33, 0x3f, 0x03, 0x1E, 0x00}, // e (101)
    {0x1C, 0x36, 0x06, 0x0f, 0x06, 0x06, 0x0F, 0x00}, // f (102)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // g (103)
    {0x07, 0x06, 0x36, 0x6E, 0x66, 0x66, 0x67, 0x00}, // h (104)
    {0x0C, 0x00, 0x0E, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // i (105)
    {0x30, 0x00, 0x30, 0x30, 0x30, 0x33, 0x33, 0x1E}, // j (106)
    {0x07, 0x06, 0x66, 0x36, 0x1E, 0x36, 0x67, 0x00}, // k (107)
    {0x0E, 0x0C, 0x0C, 0x0C, 0x0C, 0x0C, 0x1E, 0x00}, // l (108)
    {0x00, 0x00, 0x33, 0x7F, 0x7F, 0x6B, 0x63, 0x00}, // m (109)
    {0x00, 0x00, 0x1F, 0x33, 0x33, 0x33, 0x33, 0x00}, // n (110)
    {0x00, 0x00, 0x1E, 0x33, 0x33, 0x33, 0x1E, 0x00}, // o (111)
    {0x00, 0x00, 0x3B, 0x66, 0x66, 0x3E, 0x06, 0x0F}, // p (112)
    {0x00, 0x00, 0x6E, 0x33, 0x33, 0x3E, 0x30, 0x78}, // q (113)
    {0x00, 0x00, 0x3B, 0x6E, 0x66, 0x06, 0x0F, 0x00}, // r (114)
    {0x00, 0x00, 0x3E, 0x03, 0x1E, 0x30, 0x1F, 0x00}, // s (115)
    {0x08, 0x0C, 0x3E, 0x0C, 0x0C, 0x2C, 0x18, 0x00}, // t (116)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x33, 0x6E, 0x00}, // u (117)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x1E, 0x0C, 0x00}, // v (118)
    {0x00, 0x00, 0x63, 0x6B, 0x7F, 0x7F, 0x36, 0x00}, // w (119)
    {0x00, 0x00, 0x63, 0x36, 0x1C, 0x36, 0x63, 0x00}, // x (120)
    {0x00, 0x00, 0x33, 0x33, 0x33, 0x3E, 0x30, 0x1F}, // y (121)
    {0x00, 0x00, 0x3F, 0x19, 0x0C, 0x26, 0x3F, 0x00}, // z (122)
};

#define FONT_WIDTH  8
#define FONT_HEIGHT 8
#define FONT_FIRST_CHAR 32  // Space character
#define FONT_LAST_CHAR  122 // z character

static const char *TAG = "LCD_ST7789";

// Static variables for LCD state
static spi_device_handle_t lcd_spi = NULL;
static bool lcd_initialized = false;

/**
 * @brief Send command to LCD
 */
static esp_err_t lcd_send_cmd(uint8_t cmd) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    gpio_set_level(LCD_PIN_DC, 0);  // Command mode

    spi_transaction_t trans = {
        .length = 8,
        .tx_buffer = &cmd,
    };

    return spi_device_transmit(lcd_spi, &trans);
}

/**
 * @brief Send data to LCD
 */
static esp_err_t lcd_send_data(const uint8_t *data, size_t len) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (len == 0) {
        return ESP_OK;
    }

    gpio_set_level(LCD_PIN_DC, 1);  // Data mode

    spi_transaction_t trans = {
        .length = len * 8,
        .tx_buffer = data,
    };

    return spi_device_transmit(lcd_spi, &trans);
}

/**
 * @brief Set address window for drawing
 * For Waveshare ESP32-C6-LCD-1.47 (172x320 ST7789)
 */
static esp_err_t lcd_set_addr_window(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1) {
    esp_err_t ret;

    // ST7789 displays often need column and row offsets
    // For 172x320 displays, there might be offsets needed
    uint16_t col_offset = 34;  // Common offset for 172 width displays (240-172)/2 = 34
    uint16_t row_offset = 0;   // Usually no row offset for 320 height

    // Apply offsets
    x0 += col_offset;
    x1 += col_offset;
    y0 += row_offset;
    y1 += row_offset;

    // Column address set
    ret = lcd_send_cmd(0x2A);
    if (ret != ESP_OK) return ret;

    uint8_t col_data[4] = {
        (x0 >> 8) & 0xFF, x0 & 0xFF,
        (x1 >> 8) & 0xFF, x1 & 0xFF
    };
    ret = lcd_send_data(col_data, 4);
    if (ret != ESP_OK) return ret;

    // Row address set
    ret = lcd_send_cmd(0x2B);
    if (ret != ESP_OK) return ret;

    uint8_t row_data[4] = {
        (y0 >> 8) & 0xFF, y0 & 0xFF,
        (y1 >> 8) & 0xFF, y1 & 0xFF
    };
    ret = lcd_send_data(row_data, 4);
    if (ret != ESP_OK) return ret;

    // Memory write
    return lcd_send_cmd(0x2C);
}

lcd_config_t lcd_get_default_config(void) {
    lcd_config_t config = {
        .pin_mosi = LCD_PIN_MOSI,
        .pin_sclk = LCD_PIN_SCLK,
        .pin_cs = LCD_PIN_CS,
        .pin_dc = LCD_PIN_DC,
        .pin_rst = LCD_PIN_RST,
        .pin_bl = LCD_PIN_BL,
        .spi_host = LCD_SPI_HOST,
    };
    return config;
}

esp_err_t lcd_init(const lcd_config_t *config) {
    esp_err_t ret;
    
    ESP_LOGI(TAG, "Initializing ST7789 LCD...");
    
    // Configure GPIO pins
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << config->pin_dc) | (1ULL << config->pin_rst),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    gpio_config(&io_conf);
    
    // Configure SPI bus
    spi_bus_config_t buscfg = {
        .mosi_io_num = config->pin_mosi,
        .miso_io_num = -1,  // Not used for display
        .sclk_io_num = config->pin_sclk,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_WIDTH * LCD_HEIGHT * 2,
    };
    
    ret = spi_bus_initialize(config->spi_host, &buscfg, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK && ret != ESP_ERR_INVALID_STATE) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Configure SPI device
    spi_device_interface_config_t devcfg = {
        .clock_speed_hz = LCD_SPI_FREQ_HZ,
        .mode = 0,
        .spics_io_num = config->pin_cs,
        .queue_size = 7,
    };
    
    ret = spi_bus_add_device(config->spi_host, &devcfg, &lcd_spi);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to add SPI device: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Hardware reset
    gpio_set_level(config->pin_rst, 0);
    vTaskDelay(pdMS_TO_TICKS(100));
    gpio_set_level(config->pin_rst, 1);
    vTaskDelay(pdMS_TO_TICKS(100));
    
    lcd_initialized = true;

    // ST7789 initialization sequence
    ESP_LOGI(TAG, "Sending ST7789 initialization commands...");

    // Software reset
    lcd_send_cmd(0x01);
    vTaskDelay(pdMS_TO_TICKS(150));

    // Sleep out
    lcd_send_cmd(0x11);
    vTaskDelay(pdMS_TO_TICKS(120));

    // Color mode - 16bit/pixel (RGB565)
    lcd_send_cmd(0x3A);
    uint8_t color_mode = 0x05;  // 16-bit color
    lcd_send_data(&color_mode, 1);

    // Memory access control - Critical for proper orientation
    // For Waveshare ESP32-C6-LCD-1.47 (172x320 ST7789)
    lcd_send_cmd(0x36);
    uint8_t madctl = 0x00;
    // Based on research: Waveshare displays often need specific orientation
    // Try portrait mode with proper color order
    madctl = 0x00;  // Normal orientation, RGB order (try this first)
    lcd_send_data(&madctl, 1);

    // Porch setting - Important for ST7789
    lcd_send_cmd(0xB2);
    uint8_t porch_data[5] = {0x0C, 0x0C, 0x00, 0x33, 0x33};
    lcd_send_data(porch_data, 5);

    // Gate control
    lcd_send_cmd(0xB7);
    uint8_t gate_ctrl = 0x35;
    lcd_send_data(&gate_ctrl, 1);

    // VCOM setting
    lcd_send_cmd(0xBB);
    uint8_t vcom = 0x19;
    lcd_send_data(&vcom, 1);

    // LCM control
    lcd_send_cmd(0xC0);
    uint8_t lcm_ctrl = 0x2C;
    lcd_send_data(&lcm_ctrl, 1);

    // VDV and VRH command enable
    lcd_send_cmd(0xC2);
    uint8_t vdv_vrh = 0x01;
    lcd_send_data(&vdv_vrh, 1);

    // VRH set
    lcd_send_cmd(0xC3);
    uint8_t vrh = 0x12;
    lcd_send_data(&vrh, 1);

    // VDV set
    lcd_send_cmd(0xC4);
    uint8_t vdv = 0x20;
    lcd_send_data(&vdv, 1);

    // Frame rate control
    lcd_send_cmd(0xC6);
    uint8_t frame_rate = 0x0F;
    lcd_send_data(&frame_rate, 1);

    // Power control 1
    lcd_send_cmd(0xD0);
    uint8_t pwr_ctrl[2] = {0xA4, 0xA1};
    lcd_send_data(pwr_ctrl, 2);

    // Positive voltage gamma control
    lcd_send_cmd(0xE0);
    uint8_t gamma_pos[14] = {0xD0, 0x04, 0x0D, 0x11, 0x13, 0x2B, 0x3F, 0x54, 0x4C, 0x18, 0x0D, 0x0B, 0x1F, 0x23};
    lcd_send_data(gamma_pos, 14);

    // Negative voltage gamma control
    lcd_send_cmd(0xE1);
    uint8_t gamma_neg[14] = {0xD0, 0x04, 0x0C, 0x11, 0x13, 0x2C, 0x3F, 0x44, 0x51, 0x2F, 0x1F, 0x1F, 0x20, 0x23};
    lcd_send_data(gamma_neg, 14);

    // Display inversion on (can help with refresh patterns)
    lcd_send_cmd(0x21);

    // Normal display mode
    lcd_send_cmd(0x13);

    // Tearing effect line off (disable tearing effect)
    lcd_send_cmd(0x34);

    // Display on
    lcd_send_cmd(0x29);
    vTaskDelay(pdMS_TO_TICKS(100));

    ESP_LOGI(TAG, "ST7789 LCD initialized successfully");
    return ESP_OK;
}

esp_err_t lcd_set_backlight(uint8_t brightness) {
    if (brightness > 100) {
        brightness = 100;
    }
    
    // Configure PWM for backlight control
    ledc_timer_config_t ledc_timer = {
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .clk_cfg = LEDC_AUTO_CLK,
    };
    ledc_timer_config(&ledc_timer);
    
    ledc_channel_config_t ledc_channel = {
        .channel = LEDC_CHANNEL_0,
        .duty = (brightness * 255) / 100,
        .gpio_num = LCD_PIN_BL,
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_sel = LEDC_TIMER_0,
    };
    ledc_channel_config(&ledc_channel);
    
    return ESP_OK;
}

esp_err_t lcd_fill_screen(uint16_t color) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Filling screen with color 0x%04X", color);

    // Set address window to entire screen
    esp_err_t ret = lcd_set_addr_window(0, 0, LCD_WIDTH - 1, LCD_HEIGHT - 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set address window: %s", esp_err_to_name(ret));
        return ret;
    }

    // Convert color to big-endian format for transmission
    uint8_t color_data[2] = {
        (color >> 8) & 0xFF,  // High byte
        color & 0xFF          // Low byte
    };

    // Send color data for all pixels
    uint32_t total_pixels = LCD_WIDTH * LCD_HEIGHT;

    // Use maximum chunk size for fastest possible transfer
    const uint32_t chunk_size = 8192;  // Even larger chunks for maximum speed
    uint8_t *chunk_buffer = malloc(chunk_size * 2);  // 2 bytes per pixel

    if (!chunk_buffer) {
        ESP_LOGE(TAG, "Failed to allocate memory for screen fill");
        return ESP_ERR_NO_MEM;
    }

    // Fill chunk buffer with the color
    for (uint32_t i = 0; i < chunk_size; i++) {
        chunk_buffer[i * 2] = color_data[0];
        chunk_buffer[i * 2 + 1] = color_data[1];
    }

    // Send chunks as fast as possible - no delays
    uint32_t remaining_pixels = total_pixels;

    while (remaining_pixels > 0) {
        uint32_t pixels_to_send = (remaining_pixels > chunk_size) ? chunk_size : remaining_pixels;
        ret = lcd_send_data(chunk_buffer, pixels_to_send * 2);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send pixel data: %s", esp_err_to_name(ret));
            free(chunk_buffer);
            return ret;
        }
        remaining_pixels -= pixels_to_send;
    }

    free(chunk_buffer);
    ESP_LOGI(TAG, "Screen fill completed successfully");
    return ESP_OK;
}

esp_err_t lcd_fill_safe_area(uint16_t color) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Filling safe area with color 0x%04X (avoiding bezel)", color);

    // Fill only the safe area, avoiding curved edges
    return lcd_draw_rectangle(LCD_SAFE_X_START, LCD_SAFE_Y_START,
                             LCD_SAFE_WIDTH, LCD_SAFE_HEIGHT, color);
}

esp_err_t lcd_draw_safe_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // Check bounds within safe area
    if (x + width > LCD_SAFE_WIDTH || y + height > LCD_SAFE_HEIGHT) {
        ESP_LOGE(TAG, "Safe rectangle (%d,%d) size %dx%d exceeds safe area bounds (%dx%d)",
                 x, y, width, height, LCD_SAFE_WIDTH, LCD_SAFE_HEIGHT);
        return ESP_ERR_INVALID_ARG;
    }

    // Convert safe area coordinates to absolute screen coordinates
    uint16_t abs_x = LCD_SAFE_X_START + x;
    uint16_t abs_y = LCD_SAFE_Y_START + y;

    ESP_LOGI(TAG, "Drawing safe rectangle: safe(%d,%d) -> screen(%d,%d) size %dx%d",
             x, y, abs_x, abs_y, width, height);

    // Draw using absolute coordinates
    return lcd_draw_rectangle(abs_x, abs_y, width, height, color);
}

esp_err_t lcd_draw_char(uint16_t x, uint16_t y, char ch, uint16_t fg_color, uint16_t bg_color) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // Check if character is in supported range
    if (ch < FONT_FIRST_CHAR || ch > FONT_LAST_CHAR) {
        ch = ' '; // Default to space for unsupported characters
    }

    // Check bounds
    if (x + FONT_WIDTH > LCD_WIDTH || y + FONT_HEIGHT > LCD_HEIGHT) {
        return ESP_ERR_INVALID_ARG;
    }

    // Get font data for this character
    const uint8_t *char_data = font_8x8[ch - FONT_FIRST_CHAR];

    // Convert colors to big-endian format
    uint8_t fg_data[2] = {(fg_color >> 8) & 0xFF, fg_color & 0xFF};
    uint8_t bg_data[2] = {(bg_color >> 8) & 0xFF, bg_color & 0xFF};

    // Create buffer for entire character (8x8 pixels = 128 bytes)
    uint8_t char_buffer[FONT_WIDTH * FONT_HEIGHT * 2];

    // Fill buffer with character bitmap
    int buffer_index = 0;
    for (int row = 0; row < FONT_HEIGHT; row++) {
        uint8_t row_data = char_data[row];
        for (int col = 0; col < FONT_WIDTH; col++) {
            // Check if pixel should be foreground or background
            bool is_fg = (row_data & (0x01 << col)) != 0;

            // Add pixel color to buffer
            char_buffer[buffer_index++] = is_fg ? fg_data[0] : bg_data[0];
            char_buffer[buffer_index++] = is_fg ? fg_data[1] : bg_data[1];
        }
    }

    // Set address window for character
    esp_err_t ret = lcd_set_addr_window(x, y, x + FONT_WIDTH - 1, y + FONT_HEIGHT - 1);
    if (ret != ESP_OK) {
        return ret;
    }

    // Send entire character in one transfer
    return lcd_send_data(char_buffer, sizeof(char_buffer));
}

esp_err_t lcd_draw_string(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!str) {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t current_x = x;

    while (*str) {
        // Check if we have space for another character
        if (current_x + FONT_WIDTH > LCD_WIDTH) {
            break; // Stop if we would go off screen
        }

        esp_err_t ret = lcd_draw_char(current_x, y, *str, fg_color, bg_color);
        if (ret != ESP_OK) {
            return ret;
        }

        current_x += FONT_WIDTH;
        str++;
    }

    return ESP_OK;
}

esp_err_t lcd_draw_safe_string(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    // Check bounds within safe area
    if (x >= LCD_SAFE_WIDTH || y >= LCD_SAFE_HEIGHT) {
        return ESP_ERR_INVALID_ARG;
    }

    // Convert safe area coordinates to absolute screen coordinates
    uint16_t abs_x = LCD_SAFE_X_START + x;
    uint16_t abs_y = LCD_SAFE_Y_START + y;

    // Calculate maximum string length that fits in safe area
    uint16_t max_chars = (LCD_SAFE_WIDTH - x) / FONT_WIDTH;

    // Draw string with length limit
    uint16_t current_x = abs_x;
    uint16_t char_count = 0;

    while (*str && char_count < max_chars) {
        esp_err_t ret = lcd_draw_char(current_x, abs_y, *str, fg_color, bg_color);
        if (ret != ESP_OK) {
            return ret;
        }

        current_x += FONT_WIDTH;
        char_count++;
        str++;
    }

    return ESP_OK;
}

esp_err_t lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return ESP_ERR_INVALID_ARG;
    }

    // Set address window for single pixel
    esp_err_t ret = lcd_set_addr_window(x, y, x, y);
    if (ret != ESP_OK) {
        return ret;
    }

    // Send pixel color data
    uint8_t color_data[2] = {
        (color >> 8) & 0xFF,  // High byte
        color & 0xFF          // Low byte
    };

    return lcd_send_data(color_data, 2);
}

esp_err_t lcd_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (x + width > LCD_WIDTH || y + height > LCD_HEIGHT) {
        return ESP_ERR_INVALID_ARG;
    }

    ESP_LOGI(TAG, "Drawing rectangle at (%d,%d) size %dx%d with color 0x%04X",
             x, y, width, height, color);

    // Set address window for rectangle
    esp_err_t ret = lcd_set_addr_window(x, y, x + width - 1, y + height - 1);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set address window for rectangle");
        return ret;
    }

    // Convert color to big-endian format
    uint8_t color_data[2] = {
        (color >> 8) & 0xFF,  // High byte
        color & 0xFF          // Low byte
    };

    // Send color data for all pixels in rectangle
    uint32_t total_pixels = width * height;

    // For small rectangles, send pixel by pixel
    // For larger rectangles, use chunked approach like fill_screen
    if (total_pixels <= 512) {
        // Small rectangle - send each pixel
        for (uint32_t i = 0; i < total_pixels; i++) {
            ret = lcd_send_data(color_data, 2);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to send rectangle pixel data");
                return ret;
            }
        }
    } else {
        // Large rectangle - use chunked approach
        const uint32_t chunk_size = 512;
        uint8_t *chunk_buffer = malloc(chunk_size * 2);

        if (!chunk_buffer) {
            ESP_LOGE(TAG, "Failed to allocate memory for rectangle");
            return ESP_ERR_NO_MEM;
        }

        // Fill chunk buffer
        for (uint32_t i = 0; i < chunk_size; i++) {
            chunk_buffer[i * 2] = color_data[0];
            chunk_buffer[i * 2 + 1] = color_data[1];
        }

        // Send chunks
        uint32_t remaining_pixels = total_pixels;
        while (remaining_pixels > 0) {
            uint32_t pixels_to_send = (remaining_pixels > chunk_size) ? chunk_size : remaining_pixels;
            ret = lcd_send_data(chunk_buffer, pixels_to_send * 2);
            if (ret != ESP_OK) {
                ESP_LOGE(TAG, "Failed to send rectangle chunk data");
                free(chunk_buffer);
                return ret;
            }
            remaining_pixels -= pixels_to_send;
        }

        free(chunk_buffer);
    }

    ESP_LOGI(TAG, "Rectangle drawing completed successfully");
    return ESP_OK;
}

esp_err_t lcd_draw_char_scaled(uint16_t x, uint16_t y, char ch, uint16_t fg_color, uint16_t bg_color, uint8_t scale) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (scale == 0) {
        scale = 1; // Minimum scale
    }

    // For scale 1, use the optimized regular function
    if (scale == 1) {
        return lcd_draw_char(x, y, ch, fg_color, bg_color);
    }

    // Check if character is in supported range
    if (ch < FONT_FIRST_CHAR || ch > FONT_LAST_CHAR) {
        ch = ' '; // Default to space for unsupported characters
    }

    // Check bounds
    uint16_t char_width = FONT_WIDTH * scale;
    uint16_t char_height = FONT_HEIGHT * scale;
    if (x + char_width > LCD_WIDTH || y + char_height > LCD_HEIGHT) {
        return ESP_ERR_INVALID_ARG;
    }

    // Get font data for this character
    const uint8_t *char_data = font_8x8[ch - FONT_FIRST_CHAR];

    // Convert colors to big-endian format
    uint8_t fg_data[2] = {(fg_color >> 8) & 0xFF, fg_color & 0xFF};
    uint8_t bg_data[2] = {(bg_color >> 8) & 0xFF, bg_color & 0xFF};

    // Create buffer for scaled character
    uint32_t buffer_size = char_width * char_height * 2; // 2 bytes per pixel
    uint8_t *char_buffer = malloc(buffer_size);

    if (!char_buffer) {
        return ESP_ERR_NO_MEM;
    }

    // Fill buffer with scaled character bitmap
    int buffer_index = 0;
    for (int row = 0; row < FONT_HEIGHT; row++) {
        uint8_t row_data = char_data[row];

        // Repeat each row 'scale' times
        for (uint8_t sy = 0; sy < scale; sy++) {
            for (int col = 0; col < FONT_WIDTH; col++) {
                // Check if pixel should be foreground or background
                bool is_fg = (row_data & (0x01 << col)) != 0;

                // Repeat each pixel 'scale' times horizontally
                for (uint8_t sx = 0; sx < scale; sx++) {
                    char_buffer[buffer_index++] = is_fg ? fg_data[0] : bg_data[0];
                    char_buffer[buffer_index++] = is_fg ? fg_data[1] : bg_data[1];
                }
            }
        }
    }

    // Set address window for scaled character
    esp_err_t ret = lcd_set_addr_window(x, y, x + char_width - 1, y + char_height - 1);
    if (ret != ESP_OK) {
        free(char_buffer);
        return ret;
    }

    // Send entire scaled character in one transfer
    ret = lcd_send_data(char_buffer, buffer_size);
    free(char_buffer);

    return ret;
}

esp_err_t lcd_draw_string_scaled(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (!str || scale == 0) {
        return ESP_ERR_INVALID_ARG;
    }

    uint16_t current_x = x;
    uint16_t char_width = FONT_WIDTH * scale;

    while (*str) {
        // Check if we have space for another character
        if (current_x + char_width > LCD_WIDTH) {
            break; // Stop if we would go off screen
        }

        esp_err_t ret = lcd_draw_char_scaled(current_x, y, *str, fg_color, bg_color, scale);
        if (ret != ESP_OK) {
            return ret;
        }

        current_x += char_width;
        str++;
    }

    return ESP_OK;
}

esp_err_t lcd_draw_safe_string_scaled(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }

    if (scale == 0) {
        scale = 1;
    }

    // Check bounds within safe area
    uint16_t char_height = FONT_HEIGHT * scale;
    if (x >= LCD_SAFE_WIDTH || y + char_height > LCD_SAFE_HEIGHT) {
        return ESP_ERR_INVALID_ARG;
    }

    // Convert safe area coordinates to absolute screen coordinates
    uint16_t abs_x = LCD_SAFE_X_START + x;
    uint16_t abs_y = LCD_SAFE_Y_START + y;

    // Calculate maximum string length that fits in safe area
    uint16_t char_width = FONT_WIDTH * scale;
    uint16_t max_chars = (LCD_SAFE_WIDTH - x) / char_width;

    // Draw string with length limit
    uint16_t current_x = abs_x;
    uint16_t char_count = 0;

    while (*str && char_count < max_chars) {
        esp_err_t ret = lcd_draw_char_scaled(current_x, abs_y, *str, fg_color, bg_color, scale);
        if (ret != ESP_OK) {
            return ret;
        }

        current_x += char_width;
        char_count++;
        str++;
    }

    return ESP_OK;
}

esp_err_t lcd_draw_safe_text_wrapped(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale) {
    return lcd_draw_safe_text_wrapped_ex(x, y, str, fg_color, bg_color, scale, NULL);
}

esp_err_t lcd_draw_safe_text_wrapped_ex(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color, uint8_t scale, lcd_text_bounds_t *bounds) {
    if (!lcd_initialized || !str) {
        return ESP_ERR_INVALID_STATE;
    }

    if (scale == 0) {
        scale = 1;
    }

    uint16_t char_width = FONT_WIDTH * scale;
    uint16_t char_height = FONT_HEIGHT * scale;
    uint16_t line_spacing = char_height + 2; // Add 2 pixels between lines

    uint16_t current_x = x;
    uint16_t current_y = y;
    uint16_t start_y = y;
    uint16_t max_x = x; // Track maximum X position reached
    uint16_t line_count = 1; // Start with 1 line

    const char *current_pos = str;

    while (*current_pos) {
        // Check for explicit line break
        if (*current_pos == '\n') {
            // Update max_x if current line is longer
            if (current_x > max_x) {
                max_x = current_x;
            }

            current_x = x;
            current_y += line_spacing;
            line_count++;
            current_pos++;

            // Check if we're still within safe area
            if (current_y + char_height > LCD_SAFE_HEIGHT) {
                break; // Stop if we would go off screen
            }
            continue;
        }

        // Check if we need to wrap to next line
        if (current_x + char_width > LCD_SAFE_WIDTH) {
            // Update max_x before wrapping
            if (current_x > max_x) {
                max_x = current_x;
            }

            current_x = x;
            current_y += line_spacing;
            line_count++;

            // Check if we're still within safe area
            if (current_y + char_height > LCD_SAFE_HEIGHT) {
                break; // Stop if we would go off screen
            }
        }

        // Draw the character
        esp_err_t ret = lcd_draw_char_scaled(LCD_SAFE_X_START + current_x, LCD_SAFE_Y_START + current_y,
                                           *current_pos, fg_color, bg_color, scale);
        if (ret != ESP_OK) {
            return ret;
        }

        current_x += char_width;
        current_pos++;
    }

    // Update max_x for the last line
    if (current_x > max_x) {
        max_x = current_x;
    }

    // Fill bounds structure if provided
    if (bounds) {
        bounds->width = max_x - x;
        bounds->height = (current_y - start_y) + char_height;
        bounds->lines = line_count;
    }

    return ESP_OK;
}

esp_err_t lcd_printf_safe(uint16_t x, uint16_t y, uint16_t fg_color, uint16_t bg_color, uint8_t scale, const char *format, ...) {
    if (!format) {
        return ESP_ERR_INVALID_ARG;
    }

    // Create buffer for formatted string
    char buffer[256]; // Adjust size as needed

    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len < 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Ensure null termination
    if (len >= sizeof(buffer)) {
        buffer[sizeof(buffer) - 1] = '\0';
    }

    return lcd_draw_safe_text_wrapped(x, y, buffer, fg_color, bg_color, scale);
}

esp_err_t lcd_printf_safe_ex(uint16_t x, uint16_t y, uint16_t fg_color, uint16_t bg_color, uint8_t scale, lcd_text_bounds_t *bounds, const char *format, ...) {
    if (!format) {
        return ESP_ERR_INVALID_ARG;
    }

    // Create buffer for formatted string
    char buffer[256]; // Adjust size as needed

    va_list args;
    va_start(args, format);
    int len = vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    if (len < 0) {
        return ESP_ERR_INVALID_ARG;
    }

    // Ensure null termination
    if (len >= sizeof(buffer)) {
        buffer[sizeof(buffer) - 1] = '\0';
    }

    return lcd_draw_safe_text_wrapped_ex(x, y, buffer, fg_color, bg_color, scale, bounds);
}
