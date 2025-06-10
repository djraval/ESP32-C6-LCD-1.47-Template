/*
 * UI Manager Implementation
 *
 * High-level UI management for ESP32-C6-LCD-1.47 template
 */

#include "ui_manager.h"
#include "lvgl_driver.h"
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "UI_MANAGER";

// For multi-screen apps, store screen references:
// static lv_obj_t *main_screen = NULL;
// static lv_obj_t *settings_screen = NULL;

esp_err_t ui_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing UI manager");
    
    esp_err_t ret = lvgl_driver_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LVGL driver: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Create the main screen
    ESP_LOGI(TAG, "Creating main UI screen");
    
    // Background
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, lv_color_make(32, 32, 32), LV_PART_MAIN); // MODIFY: Background color
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    // Main panel
    lv_obj_t * panel = lv_obj_create(scr);
    lv_obj_set_size(panel, 160, 180); // MODIFY: Panel size
    lv_obj_set_style_bg_color(panel, lv_color_white(), LV_PART_MAIN); // MODIFY: Panel color
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(panel, 8, LV_PART_MAIN); // MODIFY: Corner radius
    lv_obj_set_style_border_width(panel, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, lv_color_make(64, 128, 255), LV_PART_MAIN); // MODIFY: Border color
    lv_obj_center(panel);

    // Title
    lv_obj_t * title = lv_label_create(panel);
    lv_label_set_text(title, "ESP32-C6"); // MODIFY: Title text
    lv_obj_set_style_text_color(title, lv_color_make(64, 128, 255), LV_PART_MAIN); // MODIFY: Title color
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 20);

    // Board info
    lv_obj_t * board_info = lv_label_create(panel);
    lv_label_set_text(board_info, "LCD-1.47\n172 x 320\nST7789"); // MODIFY: Board info text
    lv_obj_set_style_text_color(board_info, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_align(board_info, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(board_info, LV_ALIGN_CENTER, 0, 0);

    // Status
    lv_obj_t * status = lv_label_create(panel);
    lv_label_set_text(status, "Template Ready"); // MODIFY: Status text
    lv_obj_set_style_text_color(status, lv_color_make(0, 170, 0), LV_PART_MAIN); // MODIFY: Status color
    lv_obj_set_style_text_align(status, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(status, LV_ALIGN_BOTTOM_MID, 0, -20);

    // ADD NEW UI ELEMENTS HERE:
    // lv_obj_t * btn = lv_btn_create(panel);

    ESP_LOGI(TAG, "UI manager initialized successfully with main screen");
    return ESP_OK;
}

void ui_manager_run(void)
{
    // LVGL main loop
    while (1) {
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Add your event callbacks and additional screens here