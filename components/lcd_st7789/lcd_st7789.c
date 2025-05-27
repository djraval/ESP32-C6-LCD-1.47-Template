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
#include "../../../main/board_pins.h"

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

// Note: lcd_send_data function removed as it's not used in this placeholder implementation

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
    
    // Basic initialization sequence (simplified)
    lcd_send_cmd(0x01);  // Software reset
    vTaskDelay(pdMS_TO_TICKS(150));
    
    lcd_send_cmd(0x11);  // Sleep out
    vTaskDelay(pdMS_TO_TICKS(120));
    
    lcd_send_cmd(0x29);  // Display on
    
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
    
    // This is a placeholder implementation
    // In a full implementation, you would:
    // 1. Set column address (0x2A)
    // 2. Set row address (0x2B)
    // 3. Write to RAM (0x2C)
    // 4. Send color data for all pixels
    
    ESP_LOGI(TAG, "Fill screen with color 0x%04X (placeholder)", color);
    return ESP_OK;
}

esp_err_t lcd_draw_pixel(uint16_t x, uint16_t y, uint16_t color) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Placeholder implementation
    ESP_LOGD(TAG, "Draw pixel at (%d,%d) with color 0x%04X", x, y, color);
    return ESP_OK;
}

esp_err_t lcd_draw_rectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t color) {
    if (!lcd_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    if (x + width > LCD_WIDTH || y + height > LCD_HEIGHT) {
        return ESP_ERR_INVALID_ARG;
    }
    
    // Placeholder implementation
    ESP_LOGI(TAG, "Draw rectangle at (%d,%d) size %dx%d with color 0x%04X", 
             x, y, width, height, color);
    return ESP_OK;
}
