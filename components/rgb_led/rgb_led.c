/**
 * @file rgb_led.c
 * @brief RGB LED driver implementation
 */

#include "rgb_led.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "../../../main/board_pins.h"

static const char *TAG = "RGB_LED";

// WS2812 timing parameters (in nanoseconds)
#define WS2812_T0H_NS   350
#define WS2812_T0L_NS   800
#define WS2812_T1H_NS   700
#define WS2812_T1L_NS   600
#define WS2812_RESET_NS 50000

// Static variables
static bool rgb_led_initialized = false;
static int led_count = 1;

/**
 * @brief Convert HSV to RGB
 */
static void hsv_to_rgb(uint16_t h, uint8_t s, uint8_t v, uint8_t *r, uint8_t *g, uint8_t *b) {
    uint8_t region, remainder, p, q, t;

    if (s == 0) {
        *r = *g = *b = v;
        return;
    }

    region = h / 43;
    remainder = (h - (region * 43)) * 6;

    p = (v * (255 - s)) >> 8;
    q = (v * (255 - ((s * remainder) >> 8))) >> 8;
    t = (v * (255 - ((s * (255 - remainder)) >> 8))) >> 8;

    switch (region) {
        case 0:
            *r = v; *g = t; *b = p;
            break;
        case 1:
            *r = q; *g = v; *b = p;
            break;
        case 2:
            *r = p; *g = v; *b = t;
            break;
        case 3:
            *r = p; *g = q; *b = v;
            break;
        case 4:
            *r = t; *g = p; *b = v;
            break;
        default:
            *r = v; *g = p; *b = q;
            break;
    }
}

rgb_led_config_t rgb_led_get_default_config(void) {
    rgb_led_config_t config = {
        .gpio_pin = RGB_LED_PIN,
        .led_count = RGB_LED_COUNT,
        .rmt_channel = RGB_LED_RMT_CHANNEL,
    };
    return config;
}

esp_err_t rgb_led_init(const rgb_led_config_t *config) {
    ESP_LOGI(TAG, "Initializing RGB LED on GPIO%d...", config->gpio_pin);

    // Configure GPIO as output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << config->gpio_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO: %s", esp_err_to_name(ret));
        return ret;
    }

    // Note: This is a placeholder implementation
    // In a full implementation, you would:
    // 1. Configure RMT TX channel for WS2812 timing
    // 2. Create proper WS2812 encoder
    // 3. Handle color data transmission

    led_count = config->led_count;
    rgb_led_initialized = true;

    ESP_LOGI(TAG, "RGB LED initialized successfully (placeholder)");

    return ESP_OK;
}

esp_err_t rgb_led_set_color(uint8_t red, uint8_t green, uint8_t blue) {
    if (!rgb_led_initialized) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Placeholder implementation
    // In a full implementation, you would:
    // 1. Create WS2812 data format (GRB order)
    // 2. Use RMT to transmit the data
    // 3. Handle timing requirements
    
    ESP_LOGI(TAG, "Set RGB LED color: R=%d, G=%d, B=%d", red, green, blue);
    
    return ESP_OK;
}

esp_err_t rgb_led_set_hsv(uint16_t hue, uint8_t saturation, uint8_t value) {
    uint8_t r, g, b;
    
    if (hue > 360) hue = 360;
    if (saturation > 100) saturation = 100;
    if (value > 100) value = 100;
    
    // Convert percentages to 0-255 range
    saturation = (saturation * 255) / 100;
    value = (value * 255) / 100;
    
    hsv_to_rgb(hue, saturation, value, &r, &g, &b);
    
    return rgb_led_set_color(r, g, b);
}

esp_err_t rgb_led_off(void) {
    return rgb_led_set_color(RGB_LED_COLOR_OFF);
}
