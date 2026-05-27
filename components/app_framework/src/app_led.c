/*
 * app_led — Phase 1 implementation.
 *
 * Wraps led_hal_*. The pulse functionality is layered on top via esp_timer:
 * we keep an internal "currently pulsing" color and toggle the LED on each
 * half-period firing. Any of {set, flash, off} cancels the pulse so behavior
 * stays predictable.
 */

#include "app_led.h"

#include "led_hal.h"
#include "esp_timer.h"
#include "esp_log.h"

static const char *TAG = "app_led";

static esp_timer_handle_t s_pulse_timer;
static uint8_t s_pulse_r, s_pulse_g, s_pulse_b;
static bool    s_pulse_on;
static bool    s_pulse_active;

static void pulse_cb(void *arg)
{
    (void)arg;
    if (!s_pulse_active) {
        return;
    }
    s_pulse_on = !s_pulse_on;
    if (s_pulse_on) {
        led_hal_set_color(s_pulse_r, s_pulse_g, s_pulse_b);
    } else {
        led_hal_clear();
    }
}

static esp_err_t ensure_pulse_timer(void)
{
    if (s_pulse_timer) {
        return ESP_OK;
    }
    const esp_timer_create_args_t args = {
        .callback = pulse_cb,
        .name     = "app_led_pulse",
    };
    return esp_timer_create(&args, &s_pulse_timer);
}

static void stop_pulse(void)
{
    if (s_pulse_active) {
        s_pulse_active = false;
        esp_timer_stop(s_pulse_timer);
    }
}

esp_err_t app_led_set(uint8_t r, uint8_t g, uint8_t b)
{
    stop_pulse();
    return led_hal_set_color(r, g, b);
}

esp_err_t app_led_flash(uint8_t r, uint8_t g, uint8_t b, uint32_t ms)
{
    stop_pulse();
    return led_hal_flash(r, g, b, ms);
}

esp_err_t app_led_pulse(uint8_t r, uint8_t g, uint8_t b, uint32_t period_ms)
{
    if (period_ms == 0) {
        stop_pulse();
        return led_hal_clear();
    }

    esp_err_t err = ensure_pulse_timer();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "esp_timer_create: %s", esp_err_to_name(err));
        return err;
    }

    if (s_pulse_active) {
        esp_timer_stop(s_pulse_timer);
    }
    s_pulse_r = r; s_pulse_g = g; s_pulse_b = b;
    s_pulse_on = false;
    s_pulse_active = true;
    /* Half-period so a full on/off cycle equals period_ms. */
    uint64_t half_us = (uint64_t)period_ms * 500ULL;
    if (half_us == 0) half_us = 1;
    return esp_timer_start_periodic(s_pulse_timer, half_us);
}

esp_err_t app_led_off(void)
{
    stop_pulse();
    return led_hal_clear();
}
