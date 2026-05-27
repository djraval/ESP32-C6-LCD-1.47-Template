/*
 * app_net — Phase 1 implementation.
 *
 * Tracks online state and dispatches callbacks. In Phase 1 net_hal_init() is
 * synchronous: once it returns ESP_OK the board has an IP, so we set state
 * UP immediately after init. A future net_hal that exposes async up/down
 * events can call app_net_set_state() from its event handler — the
 * subscribers don't care where the signal comes from.
 *
 * Up to APP_NET_MAX_SUBSCRIBERS callbacks may be registered. The list is
 * static (no heap, no resize) which is appropriate for a framework where
 * the set of subscribers is known at build time.
 */

#include "app_net.h"

#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "esp_log.h"

#define APP_NET_MAX_SUBSCRIBERS 4

static const char *TAG = "app_net";

typedef struct {
    app_net_state_cb_t cb;
    void *ctx;
} subscriber_t;

static subscriber_t s_subs[APP_NET_MAX_SUBSCRIBERS];
static size_t       s_sub_count;
static app_net_state_t s_state = APP_NET_DOWN;
static SemaphoreHandle_t s_lock;

static SemaphoreHandle_t lock(void)
{
    if (!s_lock) {
        s_lock = xSemaphoreCreateMutex();
    }
    return s_lock;
}

bool app_net_is_online(void)
{
    return s_state == APP_NET_UP;
}

esp_err_t app_on_net_state(app_net_state_cb_t cb, void *ctx)
{
    if (!cb) {
        return ESP_ERR_INVALID_ARG;
    }
    SemaphoreHandle_t mu = lock();
    if (!mu) return ESP_ERR_NO_MEM;

    xSemaphoreTake(mu, portMAX_DELAY);
    if (s_sub_count >= APP_NET_MAX_SUBSCRIBERS) {
        xSemaphoreGive(mu);
        ESP_LOGE(TAG, "subscriber table full (max %d)", APP_NET_MAX_SUBSCRIBERS);
        return ESP_ERR_NO_MEM;
    }
    s_subs[s_sub_count++] = (subscriber_t){ .cb = cb, .ctx = ctx };
    app_net_state_t snapshot = s_state;
    xSemaphoreGive(mu);

    /* Fire current state once so caller doesn't need separate "first read". */
    cb(snapshot, ctx);
    return ESP_OK;
}

/* Internal: called by app_init() / future net events. */
void app_net_set_state_internal(app_net_state_t s);
void app_net_set_state_internal(app_net_state_t s)
{
    SemaphoreHandle_t mu = lock();
    if (!mu) return;

    xSemaphoreTake(mu, portMAX_DELAY);
    if (s_state == s) {
        xSemaphoreGive(mu);
        return;
    }
    s_state = s;
    /* Copy the subscriber list under the lock, then release so callbacks
     * can re-enter the API (e.g. register another callback) without
     * deadlocking. */
    subscriber_t snap[APP_NET_MAX_SUBSCRIBERS];
    size_t n = s_sub_count;
    memcpy(snap, s_subs, n * sizeof(snap[0]));
    xSemaphoreGive(mu);

    for (size_t i = 0; i < n; i++) {
        snap[i].cb(s, snap[i].ctx);
    }
}
