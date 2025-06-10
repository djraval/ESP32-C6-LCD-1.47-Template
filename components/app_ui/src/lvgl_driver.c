/*
 * LVGL Driver Implementation
 * 
 * LVGL integration and display driver for ESP32-C6-LCD-1.47
 */

#include "lvgl.h"
#include "display_hal.h"
#include "board_config.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "LVGL_DRIVER";

// LVGL display driver and buffers
static lv_disp_draw_buf_t disp_buf;
static lv_color_t buf1[LCD_H_RES * 40];
static lv_color_t buf2[LCD_H_RES * 40];
static lv_disp_drv_t disp_drv;

// LVGL display flush callback
static void lvgl_flush_cb(lv_disp_drv_t *drv, const lv_area_t *area, lv_color_t *color_map)
{
    esp_lcd_panel_handle_t panel = (esp_lcd_panel_handle_t) drv->user_data;
    esp_lcd_panel_draw_bitmap(panel, area->x1, area->y1, area->x2 + 1, area->y2 + 1, color_map);
    lv_disp_flush_ready(drv);
}

// LVGL tick timer callback
static void increase_lvgl_tick(void *arg)
{
    lv_tick_inc(2);
}

esp_err_t lvgl_driver_init(void)
{
    ESP_LOGI(TAG, "Initializing LVGL");
    
    // Initialize LVGL
    lv_init();
    
    // Initialize display buffer
    lv_disp_draw_buf_init(&disp_buf, buf1, buf2, LCD_H_RES * 40);

    // Initialize display driver
    lv_disp_drv_init(&disp_drv);
    disp_drv.hor_res = LCD_H_RES;
    disp_drv.ver_res = LCD_V_RES;
    disp_drv.flush_cb = lvgl_flush_cb;
    disp_drv.draw_buf = &disp_buf;
    disp_drv.user_data = display_hal_get_panel();
    lv_disp_drv_register(&disp_drv);

    // Start LVGL tick timer
    esp_timer_handle_t lvgl_tick_timer = NULL;
    esp_err_t ret = esp_timer_create(&(esp_timer_create_args_t){
        .callback = &increase_lvgl_tick,
        .name = "lvgl_tick"
    }, &lvgl_tick_timer);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LVGL tick timer: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_timer_start_periodic(lvgl_tick_timer, 2000);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to start LVGL tick timer: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "LVGL initialized successfully");
    return ESP_OK;
}