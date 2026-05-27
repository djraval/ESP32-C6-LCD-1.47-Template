/*
 * app_display — high-level LCD helpers.
 *
 * The framework owns the LVGL screen. App code never sees lv_obj_t* unless it
 * asks for the escape hatch app_display_content(). Three plain functions
 * cover most use cases:
 *
 *   show_status() — short line (e.g. "Online", "Connecting…")
 *   notify()      — title + body (replaces previous notification)
 *   clear()       — wipe content back to defaults
 *
 * All three are thread-safe. They marshal updates onto the LVGL task via
 * ui_manager's internal queue, so calls from MQTT / HTTP / button callbacks
 * are safe.
 */

#pragma once

#include "esp_err.h"
#include "lvgl.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Replace the status line text (short status, e.g. "Connecting…").
 *
 * @param text Null-terminated string. Truncated if longer than the status
 *             field can hold. Pass "" to blank.
 */
esp_err_t app_display_show_status(const char *text);

/**
 * @brief Replace the main notification (title + body) on the screen.
 *
 * @param title Null-terminated title; pass "" to clear.
 * @param body  Null-terminated body;  pass "" to clear.
 */
esp_err_t app_display_notify(const char *title, const char *body);

/**
 * @brief Wipe title + body back to defaults.
 */
esp_err_t app_display_clear(void);

/**
 * @brief Escape hatch — return the active LVGL screen for direct LVGL use.
 *
 * Returns the same object as lv_scr_act(). Useful when the framework helpers
 * aren't enough (charts, custom widgets, animations). The caller is
 * responsible for thread-safety — only mutate LVGL objects from inside the
 * UI task (e.g. via an esp_timer callback configured with skip_unhandled).
 */
lv_obj_t *app_display_content(void);

#ifdef __cplusplus
}
#endif
