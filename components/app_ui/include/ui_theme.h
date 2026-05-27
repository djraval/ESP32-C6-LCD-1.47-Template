/*
 * UI theme & layout tunables
 * ==========================
 *
 * Single source of truth for the notification screen's layout, colors, and
 * default text. Edit values here, run `make flash PORT=/dev/ttyACM0`, and
 * the change lands on the next boot.
 *
 * Iteration loop:
 *   1. tweak a value below
 *   2. make flash PORT=/dev/ttyACM0
 *   3. mosquitto_pub -h broker.emqx.io -t devarshi/esp32c6/notify -m "any text"
 *      (or just look at the empty-state screen if you're tuning that)
 *
 * Constraints to keep in mind:
 *   - Screen is 172 wide x 320 tall (LCD_H_RES / LCD_V_RES, set in board_config.h).
 *   - The Waveshare 1.47" has a tiny bezel; UI_SAFE_MARGIN_* below 6 px will
 *     start to clip visually even though the framebuffer is fine.
 *   - The default LVGL font is ASCII only.
 */

#pragma once

#include "lvgl.h"
#include "board_config.h"

/* ---- Safe area --------------------------------------------------------- */
/* Pixels of breathing room kept between the panel and the physical screen edge. */
#define UI_SAFE_MARGIN_X      10
#define UI_SAFE_MARGIN_Y      14

/* ---- Panel (the white card the labels sit on) -------------------------- */
#define UI_PANEL_W            (LCD_H_RES - 2 * UI_SAFE_MARGIN_X)
#define UI_PANEL_H            (LCD_V_RES - 2 * UI_SAFE_MARGIN_Y)
#define UI_PANEL_RADIUS       8
#define UI_PANEL_BORDER_W     2

/* ---- Inside-panel layout ---------------------------------------------- */
#define UI_TEXT_PAD           8                              /* horizontal padding for text */

#define UI_TITLE_TOP_Y        10                             /* offset from panel top */
#define UI_TITLE_H            28                             /* reserved vertical band for title */

#define UI_BODY_TOP_Y         (UI_TITLE_TOP_Y + UI_TITLE_H + 8) /* explicit gap below title */
#define UI_BODY_W             (UI_PANEL_W - 2 * UI_TEXT_PAD)
#define UI_BODY_H             180

#define UI_COUNTER_BOTTOM_Y   (-30)                          /* offset from panel bottom (negative) */
#define UI_STATUS_BOTTOM_Y    (-8)

/* ---- Colors ----------------------------------------------------------- */
#define UI_COLOR_BG           lv_color_make(32, 32, 32)
#define UI_COLOR_PANEL        lv_color_white()
#define UI_COLOR_BORDER       lv_color_make(64, 128, 255)
#define UI_COLOR_TITLE        lv_color_make(64, 128, 255)
#define UI_COLOR_BODY         lv_color_black()
#define UI_COLOR_COUNTER      lv_color_make(100, 100, 100)
#define UI_COLOR_STATUS       lv_color_make(0, 170, 0)

/* ---- Default text ----------------------------------------------------- */
#define UI_DEFAULT_TITLE      "Notifications"
#define UI_DEFAULT_BODY       "(waiting for messages)"
#define UI_STATUS_TEXT        "MQTT"
