/*
 * app_net — network status, read-only.
 *
 * The framework brings up WiFi itself (via net_hal in Phase 1, via BLE
 * provisioning in Phase 3). App code does not configure WiFi; it only
 * observes whether the connection is up.
 *
 * State is coarse-grained: APP_NET_UP once an IP is held, APP_NET_DOWN
 * otherwise. Multiple state callbacks may be registered.
 */

#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    APP_NET_DOWN = 0,
    APP_NET_UP   = 1,
} app_net_state_t;

typedef void (*app_net_state_cb_t)(app_net_state_t state, void *ctx);

/** @brief Current online state. */
bool app_net_is_online(void);

/**
 * @brief Register a callback fired whenever the online state changes.
 *
 * The callback is also invoked once at registration time with the current
 * state, so app code does not need separate "first read" logic.
 *
 * Callbacks run from an internal task; they must not block.
 */
esp_err_t app_on_net_state(app_net_state_cb_t cb, void *ctx);

#ifdef __cplusplus
}
#endif
