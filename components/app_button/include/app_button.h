#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_BUTTON_PRESS,            /* fires on debounced press (level goes low) */
    APP_BUTTON_RELEASE,          /* fires on debounced release */
    APP_BUTTON_LONG_PRESS,       /* fires once when held >= long_press_ms */
} app_button_event_t;

typedef void (*app_button_cb_t)(app_button_event_t ev, void *ctx);

/* Register a button. Pins are configured as input + pull-up + ANYEDGE.
 * `long_press_ms` is only used for APP_BUTTON_LONG_PRESS events; pass 0 to
 * disable. The same pin may be registered multiple times with different
 * modes — all matching callbacks fire.
 */
esp_err_t app_button_register(int gpio, app_button_event_t mode,
                              uint32_t long_press_ms,
                              app_button_cb_t cb, void *ctx);

#ifdef __cplusplus
}
#endif
