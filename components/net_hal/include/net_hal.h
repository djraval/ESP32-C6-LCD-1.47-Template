/*
 * Network HAL — WiFi station with BLE-based provisioning.
 *
 * net_hal owns the full network lifecycle:
 *   - First boot: advertises over BLE via wifi_provisioning_manager. The user
 *     uses the official "ESP BLE Provisioning" phone app to send SSID +
 *     password; creds are stored by IDF in the default WiFi NVS namespace.
 *   - Subsequent boots: reads stored creds, connects to WiFi.
 *   - net_hal_reset_credentials() erases creds (the next boot re-provisions).
 *
 * Lifecycle is asynchronous: net_hal_init() returns immediately and the
 * caller learns about state changes through a callback (net_hal_event_cb_t).
 * This lets the UI render boot/provisioning status without blocking.
 */

#pragma once

#include <stdbool.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    NET_HAL_BOOTING = 0,    /* initial state before init runs */
    NET_HAL_PROVISIONING,   /* BLE advertising, waiting for creds */
    NET_HAL_CONNECTING,     /* creds present, trying to associate */
    NET_HAL_CONNECTED,      /* IP acquired */
    NET_HAL_DISCONNECTED,   /* was connected, lost link */
    NET_HAL_FAILED,         /* persistent failure (e.g. wrong creds) */
} net_hal_state_t;

typedef struct {
    net_hal_state_t state;
    /* Populated when state == PROVISIONING. Names point to internal storage
     * valid for the lifetime of the event callback only. */
    const char *prov_service_name;   /* e.g. "PROV_1A2B" */
    const char *prov_pop;            /* proof-of-possession string */
} net_hal_event_t;

typedef void (*net_hal_event_cb_t)(const net_hal_event_t *ev, void *ctx);

typedef struct {
    net_hal_event_cb_t event_cb;
    void              *event_ctx;
} net_hal_config_t;

/**
 * @brief Start the network lifecycle. Non-blocking.
 *
 * Spawns an internal task that handles provisioning (if needed) and WiFi
 * connection. State transitions are reported via @c cfg->event_cb.
 *
 * @return ESP_OK if the task launched.
 */
esp_err_t net_hal_init(const net_hal_config_t *cfg);

/**
 * @brief Erase stored WiFi credentials, then reboot.
 *
 * Safe to call from any task. After the reboot, net_hal_init() will enter
 * PROVISIONING again.
 */
void net_hal_reset_credentials(void);

/** @brief True if the network is currently CONNECTED. */
bool net_hal_is_connected(void);

#ifdef __cplusplus
}
#endif
