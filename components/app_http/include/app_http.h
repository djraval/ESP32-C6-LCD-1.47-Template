#pragma once

#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

/* status < 0 → transport error. body may be NULL when len == 0. */
typedef void (*app_http_cb_t)(int status, const char *body, size_t len, void *ctx);

esp_err_t app_http_get(const char *url, app_http_cb_t cb, void *ctx);

esp_err_t app_http_post(const char *url,
                        const void *payload, size_t len,
                        app_http_cb_t cb, void *ctx);

#ifdef __cplusplus
}
#endif
