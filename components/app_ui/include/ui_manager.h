/*
 * UI Manager Interface
 *
 * High-level UI management for ESP32-C6-LCD-1.47 template
 *
 * To modify UI: Edit ui_manager.c - look for "MODIFY:" comments
 * To add screens: Create new functions and declare them below
 */

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    char title[32];
    char body[160];
} ui_notification_t;

/**
 * @brief Initialize UI manager and create main screen
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t ui_manager_init(void);

/**
 * @brief Run UI main loop - call this in main application loop
 */
void ui_manager_run(void);

/**
 * @brief Post a notification to be rendered by the UI task.
 *
 * Thread-safe: may be called from any task (e.g. the MQTT client task).
 * Drops the notification (returns ESP_ERR_NO_MEM) if the internal queue is full
 * rather than blocking.
 *
 * @param n Notification payload (copied into the queue).
 * @return ESP_OK on enqueue, ESP_ERR_INVALID_STATE if ui_manager_init() not run,
 *         ESP_ERR_NO_MEM if queue is full, ESP_ERR_INVALID_ARG if n is NULL.
 */
esp_err_t ui_manager_post_notification(const ui_notification_t *n);

#ifdef __cplusplus
}
#endif
