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

/**
 * @brief Initialize UI manager and create main screen
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t ui_manager_init(void);

/**
 * @brief Run UI main loop - call this in main application loop
 */
void ui_manager_run(void);

// Add your new screen functions here:
// esp_err_t ui_manager_create_settings_screen(void);
// void ui_manager_show_settings(void);

#ifdef __cplusplus
}
#endif