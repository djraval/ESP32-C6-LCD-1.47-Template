/*
 * app_http — esp_http_client wrapper.
 *
 * Each request runs on a one-shot task. The response body is collected into a
 * heap buffer (capped at CONFIG_APP_HTTP_MAX_RESP_BYTES) and handed to the
 * user callback when the request finishes.
 */

#include "app_http.h"

#include <stdlib.h>
#include <string.h>

#include "esp_log.h"
#include "esp_http_client.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "sdkconfig.h"

static const char *TAG = "app_http";

typedef struct {
    char           *url;
    void           *body;
    size_t          body_len;
    bool            is_post;
    app_http_cb_t   cb;
    void           *ctx;

    char           *resp;       /* heap, capped */
    size_t          resp_len;
    size_t          resp_cap;
} req_t;

static esp_err_t http_event(esp_http_client_event_t *evt)
{
    req_t *r = evt->user_data;
    if (evt->event_id == HTTP_EVENT_ON_DATA && evt->data && evt->data_len > 0) {
        size_t room = r->resp_cap - r->resp_len;
        size_t n = evt->data_len < room ? (size_t)evt->data_len : room;
        if (n) {
            memcpy(r->resp + r->resp_len, evt->data, n);
            r->resp_len += n;
        }
    }
    return ESP_OK;
}

static void req_task(void *arg)
{
    req_t *r = arg;

    r->resp_cap = CONFIG_APP_HTTP_MAX_RESP_BYTES;
    r->resp     = malloc(r->resp_cap);
    if (!r->resp) {
        ESP_LOGE(TAG, "OOM response buffer");
        if (r->cb) r->cb(-1, NULL, 0, r->ctx);
        goto cleanup;
    }

    esp_http_client_config_t cfg = {
        .url           = r->url,
        .event_handler = http_event,
        .user_data     = r,
        .timeout_ms    = 10000,
    };
    esp_http_client_handle_t h = esp_http_client_init(&cfg);
    if (!h) {
        if (r->cb) r->cb(-1, NULL, 0, r->ctx);
        goto cleanup;
    }

    if (r->is_post) {
        esp_http_client_set_method(h, HTTP_METHOD_POST);
        esp_http_client_set_post_field(h, r->body, r->body_len);
    }

    esp_err_t err = esp_http_client_perform(h);
    int status = (err == ESP_OK) ? esp_http_client_get_status_code(h) : -1;
    esp_http_client_cleanup(h);

    if (r->cb) r->cb(status, r->resp, r->resp_len, r->ctx);

cleanup:
    free(r->resp);
    free(r->url);
    free(r->body);
    free(r);
    vTaskDelete(NULL);
}

static esp_err_t submit(const char *url, const void *body, size_t len,
                        bool is_post, app_http_cb_t cb, void *ctx)
{
    if (!url) return ESP_ERR_INVALID_ARG;
    req_t *r = calloc(1, sizeof(*r));
    if (!r) return ESP_ERR_NO_MEM;
    r->url     = strdup(url);
    r->is_post = is_post;
    r->cb      = cb;
    r->ctx     = ctx;
    if (body && len) {
        r->body = malloc(len);
        if (!r->body) { free(r->url); free(r); return ESP_ERR_NO_MEM; }
        memcpy(r->body, body, len);
        r->body_len = len;
    }
    BaseType_t ok = xTaskCreate(req_task, "app_http",
                                CONFIG_APP_HTTP_TASK_STACK,
                                r, 4, NULL);
    if (ok != pdPASS) {
        free(r->body); free(r->url); free(r);
        return ESP_ERR_NO_MEM;
    }
    return ESP_OK;
}

esp_err_t app_http_get(const char *url, app_http_cb_t cb, void *ctx)
{
    return submit(url, NULL, 0, false, cb, ctx);
}

esp_err_t app_http_post(const char *url, const void *payload, size_t len,
                        app_http_cb_t cb, void *ctx)
{
    return submit(url, payload, len, true, cb, ctx);
}
