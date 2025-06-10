/*
 * LVGL Driver Interface
 * 
 * LVGL integration and display driver interface
 */

#pragma once

#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize LVGL graphics library
 * 
 * Sets up LVGL with display driver, buffers, and tick timer
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t lvgl_driver_init(void);

#ifdef __cplusplus
}
#endif