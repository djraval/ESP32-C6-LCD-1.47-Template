/*
 * app_kv — NVS wrapper in a dedicated namespace.
 *
 * Stays out of the way of net_hal's "net_hal" namespace and the
 * wifi_provisioning_manager's storage. Each call opens the namespace
 * itself — simpler than caching a handle and good enough for small,
 * non-hot-path app state.
 */

#include "app_kv.h"

#include "nvs.h"
#include "nvs_flash.h"

#define KV_NS "app_kv"

esp_err_t app_kv_set_str(const char *key, const char *value)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(KV_NS, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_str(h, key, value ? value : "");
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

esp_err_t app_kv_get_str(const char *key, char *buf, size_t *len)
{
    if (!len) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = nvs_open(KV_NS, NVS_READONLY, &h);
    if (err != ESP_OK) return err;
    err = nvs_get_str(h, key, buf, len);
    nvs_close(h);
    return err;
}

esp_err_t app_kv_set_blob(const char *key, const void *value, size_t len)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(KV_NS, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_set_blob(h, key, value, len);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}

esp_err_t app_kv_get_blob(const char *key, void *buf, size_t *len)
{
    if (!len) return ESP_ERR_INVALID_ARG;
    nvs_handle_t h;
    esp_err_t err = nvs_open(KV_NS, NVS_READONLY, &h);
    if (err != ESP_OK) return err;
    err = nvs_get_blob(h, key, buf, len);
    nvs_close(h);
    return err;
}

esp_err_t app_kv_erase(const char *key)
{
    nvs_handle_t h;
    esp_err_t err = nvs_open(KV_NS, NVS_READWRITE, &h);
    if (err != ESP_OK) return err;
    err = nvs_erase_key(h, key);
    if (err == ESP_OK) err = nvs_commit(h);
    nvs_close(h);
    return err;
}
