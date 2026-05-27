/*
 * app_kv — NVS-backed key/value store for app code.
 *
 * Lives in its own NVS namespace ("app_kv") so it never collides with the
 * framework's own storage (WiFi credentials, last-SSID cache,
 * provisioning data).
 */

#pragma once

#include <stddef.h>
#include "esp_err.h"

#ifdef __cplusplus
extern "C" {
#endif

esp_err_t app_kv_set_str(const char *key, const char *value);

/**
 * @brief Read a string by key.
 *
 * @param key  NVS key.
 * @param buf  Caller-supplied buffer.
 * @param len  In: capacity of @p buf. Out: bytes written (including NUL).
 *             If buf is NULL, len returns the required size.
 */
esp_err_t app_kv_get_str(const char *key, char *buf, size_t *len);

esp_err_t app_kv_set_blob(const char *key, const void *value, size_t len);
esp_err_t app_kv_get_blob(const char *key, void *buf, size_t *len);

/** @brief Remove a key. ESP_ERR_NVS_NOT_FOUND if it didn't exist. */
esp_err_t app_kv_erase(const char *key);

#ifdef __cplusplus
}
#endif
