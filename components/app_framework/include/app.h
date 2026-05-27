/*
 * App Framework — umbrella header.
 *
 * Include this single header from main/ to get the high-level API for the
 * Waveshare ESP32-C6-LCD-1.47 board. The framework owns initialization order
 * (display, LED, UI, network) and exposes a small set of primitives:
 *
 *   - app_display_*  — write text to the LCD (LVGL is hidden)
 *   - app_led_*      — RGB LED control (set / flash / pulse)
 *   - app_net_*      — network status (read-only + state callbacks)
 *   - app_kv_*       — NVS-backed key/value store for app state
 *
 * Typical usage:
 *
 *     #include "app.h"
 *
 *     static void on_net(app_net_state_t s, void *ctx) {
 *         app_display_show_status(s == APP_NET_UP ? "Online" : "Offline");
 *     }
 *
 *     void app_main(void) {
 *         app_init();
 *         app_on_net_state(on_net, NULL);
 *         app_run();      // never returns
 *     }
 */

#pragma once

#include "esp_err.h"

#include "app_display.h"
#include "app_led.h"
#include "app_net.h"
#include "app_kv.h"

#ifdef CONFIG_APP_MQTT_ENABLED
#include "app_mqtt.h"
#endif

#ifdef CONFIG_APP_TIMER_ENABLED
#include "app_timer.h"
#endif

#ifdef CONFIG_APP_HTTP_ENABLED
#include "app_http.h"
#endif

#ifdef CONFIG_APP_BUTTON_ENABLED
#include "app_button.h"
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Initialize the framework.
 *
 * Brings up display + backlight, LED driver, LVGL/UI, and starts the network
 * stack (WiFi + — for Phase 1 — MQTT via the compat shim). Network is
 * best-effort: a failure logs and continues so the UI is still usable.
 *
 * @return ESP_OK on success. Returns the first error from a subsystem that
 *         the caller must treat as fatal (display or UI init failure).
 */
esp_err_t app_init(void);

/**
 * @brief Run the framework main loop. Never returns.
 *
 * Drives LVGL and any framework-owned timers/queues. Call this last from
 * app_main() after registering callbacks.
 */
void app_run(void);

#ifdef __cplusplus
}
#endif
