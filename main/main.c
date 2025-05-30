/*
 * ESP32-C6 Template for Waveshare ESP32-C6-LCD-1.47
 *
 * Minimal LVGL implementation with custom drivers available for reference.
 * Uses ESP-IDF built-in LCD panel API + LVGL for professional UI.
 *
 * Board: Waveshare ESP32-C6-LCD-1.47 (ST7789 172x320, WS2812 RGB LED)
 * Custom drivers: components/lcd_st7789/ and components/rgb_led/ (for learning)
 */

#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_timer.h"
#include "esp_lcd_panel_io.h"
#include "esp_lcd_panel_vendor.h"
#include "esp_lcd_panel_ops.h"
#include "driver/gpio.h"
#include "driver/spi_master.h"
#include "driver/ledc.h"
#include "esp_log.h"
#include "lvgl.h"


static const char *TAG = "ESP32C6";


// Pin definitions
#define LCD_HOST        SPI2_HOST
#define PIN_SCLK        7
#define PIN_MOSI        6
#define PIN_LCD_DC      15
#define PIN_LCD_RST     21
#define PIN_LCD_CS      14
#define PIN_BK_LIGHT    22

// Display specs for ESP32-C6-LCD-1.47
#define LCD_H_RES       172
#define LCD_V_RES       320

static esp_lcd_panel_handle_t panel_handle = NULL;
static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf1[LCD_H_RES * 40];  // Increased buffer size for better performance
static lv_color_t buf2[LCD_H_RES * 40];  // Add second buffer for double buffering
static lv_disp_drv_t disp_drv;

static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel = (esp_lcd_panel_handle_t) drv->user_data;
    int offsetx1 = area->x1;
    int offsety1 = area->y1;
    int offsetx2 = area->x2;
    int offsety2 = area->y2;

    // Draw the bitmap to the LCD panel
    esp_lcd_panel_draw_bitmap(panel, offsetx1, offsety1, offsetx2 + 1, offsety2 + 1, color_map);

    // Signal LVGL that flushing is done
    lv_disp_flush_ready(drv);
}

static void increase_lvgl_tick(void *arg)
{
    lv_tick_inc(2);
}

void app_main(void)
{
    ESP_LOGI(TAG, "ESP32-C6 Template Starting...");

    // Initialize backlight
    ledc_timer_config_t ledc_timer = {
        .speed_mode = LEDC_LOW_SPEED_MODE, .timer_num = LEDC_TIMER_0,
        .duty_resolution = LEDC_TIMER_8_BIT, .freq_hz = 5000, .clk_cfg = LEDC_AUTO_CLK
    };
    ledc_timer_config(&ledc_timer);
    ledc_channel_config_t ledc_channel = {
        .speed_mode = LEDC_LOW_SPEED_MODE, .channel = LEDC_CHANNEL_0, .timer_sel = LEDC_TIMER_0,
        .intr_type = LEDC_INTR_DISABLE, .gpio_num = PIN_BK_LIGHT, .duty = 200, .hpoint = 0
    };
    ledc_channel_config(&ledc_channel);

    // Initialize SPI bus
    spi_bus_config_t buscfg = {
        .sclk_io_num = PIN_SCLK, .mosi_io_num = PIN_MOSI, .miso_io_num = -1,
        .quadwp_io_num = -1, .quadhd_io_num = -1, .max_transfer_sz = LCD_H_RES * 80 * sizeof(uint16_t),
    };
    spi_bus_initialize(LCD_HOST, &buscfg, SPI_DMA_CH_AUTO);

    // Initialize LCD panel
    esp_lcd_panel_io_handle_t io_handle = NULL;
    esp_lcd_panel_io_spi_config_t io_config = {
        .dc_gpio_num = PIN_LCD_DC,
        .cs_gpio_num = PIN_LCD_CS,
        .pclk_hz = 80 * 1000 * 1000,
        .lcd_cmd_bits = 8,
        .lcd_param_bits = 8,
        .spi_mode = 0,
        .trans_queue_depth = 10,
        .flags = {
            .dc_low_on_data = 0,  // DC high for data
        }
    };
    esp_lcd_new_panel_io_spi((esp_lcd_spi_bus_handle_t)LCD_HOST, &io_config, &io_handle);

    esp_lcd_panel_dev_config_t panel_config = {
        .reset_gpio_num = PIN_LCD_RST,
        .rgb_endian = LCD_RGB_ENDIAN_BGR,  // Back to BGR to match LVGL color swap
        .bits_per_pixel = 16,
    };
    esp_lcd_new_panel_st7789(io_handle, &panel_config, &panel_handle);

    esp_lcd_panel_reset(panel_handle);
    esp_lcd_panel_init(panel_handle);
    esp_lcd_panel_invert_color(panel_handle, true);

    // Fix display orientation and offset for ESP32-C6-LCD-1.47
    // Based on our working custom driver: 172x320 display needs column offset of 34
    esp_lcd_panel_set_gap(panel_handle, 34, 0); // Column offset for 172 width display
    // No axis swapping needed for this specific board configuration

    esp_lcd_panel_disp_on_off(panel_handle, true);

    ESP_LOGI(TAG, "Initializing LVGL...");

    // Initialize LVGL
    lv_init();

    // Initialize display buffer with double buffering for smoother rendering
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, LCD_H_RES * 40);

    // Initialize display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = panel_handle;

    // Register the display driver
    lv_disp_drv_register(&disp_drv);

    // Start LVGL tick timer
    const esp_timer_create_args_t lvgl_tick_timer_args = {
        .callback = &increase_lvgl_tick, .name = "lvgl_tick"
    };
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_timer_create(&lvgl_tick_timer_args, &lvgl_tick_timer);
    esp_timer_start_periodic(lvgl_tick_timer, 2000);

    ESP_LOGI(TAG, "Creating UI...");

    // Set background
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, LV_PART_MAIN);

    // Update display
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(500));

    // Create text label
    lv_obj_t * label = lv_label_create(lv_scr_act());
    lv_label_set_text(label, "ESP32-C6\nTemplate\nReady!");
    lv_obj_set_style_text_color(label, lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_center(label);

    ESP_LOGI(TAG, "Template ready! Custom drivers in components/ for reference.");

    // LVGL task loop
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
