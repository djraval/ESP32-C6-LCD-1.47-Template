/*
 * UI Manager Implementation
 *
 * High-level UI management for ESP32-C6-LCD-1.47 template
 */

#include "ui_manager.h"
#include "lvgl_driver.h"
#include "lvgl.h"
#include "wifi_hal.h"
#include "time_hal.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"

static const char *TAG = "UI_MANAGER";

// UI element references for dynamic updates
static lv_obj_t *wifi_status_label = NULL;
static lv_obj_t *time_label = NULL;
static lv_obj_t *ip_label = NULL;

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

    // Main panel - made taller to accommodate new elements
    lv_obj_t * panel = lv_obj_create(scr);
    lv_obj_set_size(panel, 160, 280); // MODIFY: Panel size - increased height
    lv_obj_set_style_bg_color(panel, lv_color_white(), LV_PART_MAIN); // MODIFY: Panel color
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(panel, 8, LV_PART_MAIN); // MODIFY: Corner radius
    lv_obj_set_style_border_width(panel, 2, LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, lv_color_make(64, 128, 255), LV_PART_MAIN); // MODIFY: Border color
    lv_obj_center(panel);

    // Title
    lv_obj_t * title = lv_label_create(panel);
    lv_label_set_text(title, "ESP32-C6 WiFi+NTP"); // MODIFY: Title text
    lv_obj_set_style_text_color(title, lv_color_make(64, 128, 255), LV_PART_MAIN); // MODIFY: Title color
    lv_obj_set_style_text_align(title, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 15);

    // Board info
    lv_obj_t * board_info = lv_label_create(panel);
    lv_label_set_text(board_info, "LCD-1.47 | ST7789"); // MODIFY: Board info text - made more compact
    lv_obj_set_style_text_color(board_info, lv_color_black(), LV_PART_MAIN);
    lv_obj_set_style_text_align(board_info, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(board_info, LV_ALIGN_TOP_MID, 0, 45);

    // WiFi Status
    wifi_status_label = lv_label_create(panel);
    lv_label_set_text(wifi_status_label, "WiFi: Connecting...");
    lv_obj_set_style_text_color(wifi_status_label, lv_color_make(255, 165, 0), LV_PART_MAIN); // Orange
    lv_obj_set_style_text_align(wifi_status_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(wifi_status_label, LV_ALIGN_TOP_MID, 0, 75);

    // IP Address
    ip_label = lv_label_create(panel);
    lv_label_set_text(ip_label, "IP: Not connected");
    lv_obj_set_style_text_color(ip_label, lv_color_make(128, 128, 128), LV_PART_MAIN); // Gray
    lv_obj_set_style_text_align(ip_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(ip_label, LV_ALIGN_TOP_MID, 0, 105);

    // Time Display
    time_label = lv_label_create(panel);
    lv_label_set_text(time_label, "Time: Syncing...");
    lv_obj_set_style_text_color(time_label, lv_color_make(0, 128, 0), LV_PART_MAIN); // Green
    lv_obj_set_style_text_align(time_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(time_label, LV_ALIGN_CENTER, 0, 20);

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

static void update_wifi_status(void)
{
    if (!wifi_status_label || !ip_label) return;

    wifi_status_t status = wifi_hal_get_status();
    char ip_str[16] = {0};

    switch (status) {
        case WIFI_STATUS_CONNECTING:
            lv_label_set_text(wifi_status_label, "WiFi: Connecting...");
            lv_obj_set_style_text_color(wifi_status_label, lv_color_make(255, 165, 0), LV_PART_MAIN); // Orange
            lv_label_set_text(ip_label, "IP: Not connected");
            lv_obj_set_style_text_color(ip_label, lv_color_make(128, 128, 128), LV_PART_MAIN); // Gray
            break;
        case WIFI_STATUS_CONNECTED:
            lv_label_set_text(wifi_status_label, "WiFi: Connected");
            lv_obj_set_style_text_color(wifi_status_label, lv_color_make(0, 170, 0), LV_PART_MAIN); // Green
            if (wifi_hal_get_ip_address(ip_str) == ESP_OK) {
                char ip_text[32];
                snprintf(ip_text, sizeof(ip_text), "IP: %s", ip_str);
                lv_label_set_text(ip_label, ip_text);
                lv_obj_set_style_text_color(ip_label, lv_color_make(0, 128, 0), LV_PART_MAIN); // Green
            }
            break;
        case WIFI_STATUS_FAILED:
            lv_label_set_text(wifi_status_label, "WiFi: Failed");
            lv_obj_set_style_text_color(wifi_status_label, lv_color_make(255, 0, 0), LV_PART_MAIN); // Red
            lv_label_set_text(ip_label, "IP: Connection failed");
            lv_obj_set_style_text_color(ip_label, lv_color_make(255, 0, 0), LV_PART_MAIN); // Red
            break;
        case WIFI_STATUS_DISCONNECTED:
        default:
            lv_label_set_text(wifi_status_label, "WiFi: Disconnected");
            lv_obj_set_style_text_color(wifi_status_label, lv_color_make(128, 128, 128), LV_PART_MAIN); // Gray
            lv_label_set_text(ip_label, "IP: Not connected");
            lv_obj_set_style_text_color(ip_label, lv_color_make(128, 128, 128), LV_PART_MAIN); // Gray
            break;
    }
}

static void update_time_display(void)
{
    if (!time_label) return;

    time_sync_status_t sync_status = time_hal_get_sync_status();
    char time_str[32] = {0};

    switch (sync_status) {
        case TIME_STATUS_SYNCED:
            if (time_hal_get_time_string(time_str, "%H:%M:%S") == ESP_OK) {
                char time_text[48];
                snprintf(time_text, sizeof(time_text), "Time: %s", time_str);
                lv_label_set_text(time_label, time_text);
                lv_obj_set_style_text_color(time_label, lv_color_make(0, 128, 0), LV_PART_MAIN); // Green
            }
            break;
        case TIME_STATUS_SYNCING:
            lv_label_set_text(time_label, "Time: Syncing...");
            lv_obj_set_style_text_color(time_label, lv_color_make(255, 165, 0), LV_PART_MAIN); // Orange
            break;
        case TIME_STATUS_NOT_SYNCED:
        default:
            lv_label_set_text(time_label, "Time: Not synced");
            lv_obj_set_style_text_color(time_label, lv_color_make(128, 128, 128), LV_PART_MAIN); // Gray
            break;
    }
}

void ui_manager_run(void)
{
    uint32_t update_counter = 0;

    // LVGL main loop with periodic updates
    while (1) {
        lv_timer_handler();

        // Update WiFi and time status every 100 cycles (approximately every second)
        if (++update_counter >= 100) {
            update_wifi_status();
            update_time_display();
            update_counter = 0;
        }

        vTaskDelay(pdMS_TO_TICKS(10));
    }
}

// Add your event callbacks and additional screens here