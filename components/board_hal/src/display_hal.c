/*
 * Display Hardware Abstraction Layer Implementation
 * 
 * LCD display and backlight control for ESP32-C6-LCD-1.47
 */

#include "display_hal.h"
#include "board_config.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_log.h"

static const char *TAG = "DISPLAY_HAL";

// Hardware handles
static esp_lcd_panel_handle_t panel_handle = NULL;

static esp_err_t init_backlight(void)
{
    ESP_LOGI(TAG, "Initializing backlight");
    
    esp_err_t ret = ledc_timer_config(&(ledc_timer_config_t){
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT,
        .freq_hz = 5000,
        .clk_cfg = LEDC_AUTO_CLK
    });
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC timer: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = ledc_channel_config(&(ledc_channel_config_t){
        .speed_mode = LEDC_LOW_SPEED_MODE,
        .channel = LEDC_CHANNEL_0,
        .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE,
        .gpio_num = PIN_BK_LIGHT,
        .duty = 200,
        .hpoint = 0
    });
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure LEDC channel: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

static esp_err_t init_lcd_panel(void)
{
    ESP_LOGI(TAG, "Initializing LCD panel");
    
    // SPI bus configuration
    esp_err_t ret = spi_bus_initialize(LCD_HOST, &(spi_bus_config_t){
        .sclk_io_num = PIN_SCLK,
        .mosi_io_num = PIN_MOSI,
        .miso_io_num = -1,
        .quadwp_io_num = -1,
        .quadhd_io_num = -1,
        .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t)
    }, SPI_DMA_CH_AUTO);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize SPI bus: %s", esp_err_to_name(ret));
        return ret;
    }

    // LCD panel IO configuration
    esp_lcd_panel_io_handle_t io_handle = NULL;
    ret = esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST,
        &(esp_lcd_panel_io_spi_config_t){
            .dc_gpio_num = PIN_LCD_DC,
            .cs_gpio_num = PIN_LCD_CS,
            .pclk_hz = 80000000,
            .lcd_cmd_bits = 8,
            .lcd_param_bits = 8,
            .spi_mode = 0,
            .trans_queue_depth = 10
        }, &io_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create panel IO: %s", esp_err_to_name(ret));
        return ret;
    }

    // LCD panel configuration and initialization
    ret = esp_lcd_new_panel_st7789(io_handle, &(esp_lcd_panel_dev_config_t){
        .reset_gpio_num = PIN_LCD_RST,
        .rgb_endian = LCD_RGB_ENDIAN_RGB,
        .bits_per_pixel = 16
    }, &panel_handle);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LCD panel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Initialize and configure LCD
    ESP_ERROR_CHECK(esp_lcd_panel_reset(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_init(panel_handle));
    ESP_ERROR_CHECK(esp_lcd_panel_invert_color(panel_handle, true));
    ESP_ERROR_CHECK(esp_lcd_panel_set_gap(panel_handle, 34, 0)); // Column offset for 172 width display
    ESP_ERROR_CHECK(esp_lcd_panel_disp_on_off(panel_handle, true));

    ESP_LOGI(TAG, "LCD panel initialized successfully");
    return ESP_OK;
}

esp_err_t display_hal_init(void)
{
    ESP_LOGI(TAG, "Initializing display hardware");
    
    esp_err_t ret = init_backlight();
    if (ret != ESP_OK) {
        return ret;
    }
    
    ret = init_lcd_panel();
    if (ret != ESP_OK) {
        return ret;
    }
    
    ESP_LOGI(TAG, "Display hardware initialized successfully");
    return ESP_OK;
}

esp_lcd_panel_handle_t display_hal_get_panel(void)
{
    return panel_handle;
}

esp_err_t display_hal_set_backlight(uint8_t brightness)
{
    esp_err_t ret = ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, brightness);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set backlight duty: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to update backlight duty: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}