/*
 * app_display — Phase 1 implementation.
 *
 * Thin shim over ui_manager. The mapping:
 *   - notify(title, body)  → ui_notification_t through the existing queue
 *   - show_status(text)    → for now, also rendered as a notification body so
 *                            users can see it land on the LCD without forcing
 *                            a ui_manager refactor in Phase 1
 *   - clear()              → notify("", "")
 *
 * A proper 3-slot layout (status bar + content + hint) is a later iteration;
 * the API above doesn't need to change for that.
 */

#include "app_display.h"

#include <string.h>
#include "ui_manager.h"
#include "lvgl.h"

esp_err_t app_display_show_status(const char *text)
{
    ui_notification_t n = { 0 };
    /* Use the body slot for the status line in Phase 1; title stays blank
     * so the screen reads cleanly. */
    if (text) {
        strncpy(n.body, text, sizeof(n.body) - 1);
    }
    return ui_manager_post_notification(&n);
}

esp_err_t app_display_notify(const char *title, const char *body)
{
    ui_notification_t n = { 0 };
    if (title) {
        strncpy(n.title, title, sizeof(n.title) - 1);
    }
    if (body) {
        strncpy(n.body, body, sizeof(n.body) - 1);
    }
    return ui_manager_post_notification(&n);
}

esp_err_t app_display_clear(void)
{
    ui_notification_t n = { 0 };
    return ui_manager_post_notification(&n);
}

lv_obj_t *app_display_content(void)
{
    return lv_scr_act();
}
