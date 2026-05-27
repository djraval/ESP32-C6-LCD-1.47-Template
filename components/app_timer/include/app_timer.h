#pragma once

#include <stdint.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*app_timer_cb_t)(void *ctx);

typedef struct app_timer *app_timer_handle_t;

/* Fire cb every `period_ms` on a worker task (NOT in ISR). */
esp_err_t app_timer_every(uint32_t period_ms, app_timer_cb_t cb, void *ctx,
                          app_timer_handle_t *out);

/* Cancel a recurring timer. */
esp_err_t app_timer_cancel(app_timer_handle_t h);

/* Post a one-shot callback to the worker (no delay). */
esp_err_t app_defer(app_timer_cb_t cb, void *ctx);

#ifdef __cplusplus
}
#endif
