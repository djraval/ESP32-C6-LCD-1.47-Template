/*
 * UI Manager Implementation
 *
 * High-level UI management for ESP32-C6-LCD-1.47 template
 */

#include "ui_manager.h"
#include "ui_theme.h"
#include "lvgl_driver.h"
#include "lvgl.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include "esp_log.h"
#include <string.h>
#include <stdio.h>

static const char *TAG = "UI_MANAGER";

#define NOTIF_QUEUE_LEN     8

// LVGL objects updated by the notification path. Owned by the LVGL/UI task.
static lv_obj_t *s_title_label;
static lv_obj_t *s_body_label;
static lv_obj_t *s_status_label;
static lv_obj_t *s_counter_label;

static QueueHandle_t s_notif_queue;
static uint32_t s_notif_count;

esp_err_t ui_manager_init(void)
{
    ESP_LOGI(TAG, "Initializing UI manager");

    esp_err_t ret = lvgl_driver_init();
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to initialize LVGL driver: %s", esp_err_to_name(ret));
        return ret;
    }

    s_notif_queue = xQueueCreate(NOTIF_QUEUE_LEN, sizeof(ui_notification_t));
    if (!s_notif_queue) {
        ESP_LOGE(TAG, "Failed to create notification queue");
        return ESP_ERR_NO_MEM;
    }

    // Create the main screen.
    // MODIFY: layout, colors, default text → edit components/app_ui/include/ui_theme.h
    ESP_LOGI(TAG, "Creating main UI screen");

    // Background
    lv_obj_t * scr = lv_scr_act();
    lv_obj_set_style_bg_color(scr, UI_COLOR_BG, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, LV_PART_MAIN);

    // Main panel
    lv_obj_t * panel = lv_obj_create(scr);
    lv_obj_set_size(panel, UI_PANEL_W, UI_PANEL_H);
    lv_obj_set_style_bg_color(panel, UI_COLOR_PANEL, LV_PART_MAIN);
    lv_obj_set_style_bg_opa(panel, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_radius(panel, UI_PANEL_RADIUS, LV_PART_MAIN);
    lv_obj_set_style_border_width(panel, UI_PANEL_BORDER_W, LV_PART_MAIN);
    lv_obj_set_style_border_color(panel, UI_COLOR_BORDER, LV_PART_MAIN);
    lv_obj_center(panel);

    // Title — anchored to panel top
    s_title_label = lv_label_create(panel);
    lv_label_set_text(s_title_label, UI_DEFAULT_TITLE);
    lv_obj_set_style_text_color(s_title_label, UI_COLOR_TITLE, LV_PART_MAIN);
    lv_obj_set_style_text_align(s_title_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(s_title_label, LV_ALIGN_TOP_MID, 0, UI_TITLE_TOP_Y);

    // Body — anchored *below* the title with an explicit gap, so wrapping
    // text grows downward into UI_BODY_H and can't collide with the title.
    s_body_label = lv_label_create(panel);
    lv_label_set_long_mode(s_body_label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(s_body_label, UI_BODY_W);
    lv_label_set_text(s_body_label, UI_DEFAULT_BODY);
    lv_obj_set_style_text_color(s_body_label, UI_COLOR_BODY, LV_PART_MAIN);
    lv_obj_set_style_text_align(s_body_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(s_body_label, LV_ALIGN_TOP_MID, 0, UI_BODY_TOP_Y);

    // Counter
    s_counter_label = lv_label_create(panel);
    lv_label_set_text(s_counter_label, "received: 0");
    lv_obj_set_style_text_color(s_counter_label, UI_COLOR_COUNTER, LV_PART_MAIN);
    lv_obj_set_style_text_align(s_counter_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(s_counter_label, LV_ALIGN_BOTTOM_MID, 0, UI_COUNTER_BOTTOM_Y);

    // Status — connection/system state, updated from outside via a future hook if needed
    s_status_label = lv_label_create(panel);
    lv_label_set_text(s_status_label, UI_STATUS_TEXT);
    lv_obj_set_style_text_color(s_status_label, UI_COLOR_STATUS, LV_PART_MAIN);
    lv_obj_set_style_text_align(s_status_label, LV_TEXT_ALIGN_CENTER, LV_PART_MAIN);
    lv_obj_align(s_status_label, LV_ALIGN_BOTTOM_MID, 0, UI_STATUS_BOTTOM_Y);

    ESP_LOGI(TAG, "UI manager initialized successfully with main screen");
    return ESP_OK;
}

esp_err_t ui_manager_post_notification(const ui_notification_t *n)
{
    if (!n) {
        return ESP_ERR_INVALID_ARG;
    }
    if (!s_notif_queue) {
        return ESP_ERR_INVALID_STATE;
    }
    if (xQueueSend(s_notif_queue, n, 0) != pdTRUE) {
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

static void drain_notifications(void)
{
    ui_notification_t n;
    bool got_any = false;
    // Drain everything pending; only render the most recent.
    while (xQueueReceive(s_notif_queue, &n, 0) == pdTRUE) {
        s_notif_count++;
        got_any = true;
    }
    if (!got_any) {
        return;
    }

    lv_label_set_text(s_title_label, n.title);
    lv_label_set_text(s_body_label, n.body);

    char counter_buf[32];
    snprintf(counter_buf, sizeof(counter_buf), "received: %lu", (unsigned long)s_notif_count);
    lv_label_set_text(s_counter_label, counter_buf);
}

void ui_manager_run(void)
{
    while (1) {
        drain_notifications();
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(10));
    }
}
