/**
 * @file rgb_led.c
 * @brief RGB LED driver implementation for WS2812-style RGB LED
 */

#include "rgb_led.h"
#include "esp_log.h"
#include "driver/rmt_tx.h"
#include "driver/rmt_encoder.h"
#include "driver/gpio.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <stdlib.h>
#include <string.h>
// Pin definitions for ESP32-C6-LCD-1.47
#define RGB_LED_PIN         8
#define RGB_LED_COUNT       1
#define RGB_LED_RMT_CHANNEL 0

static const char *TAG = "RGB_LED";

// WS2812 timing parameters (in nanoseconds) - Updated to standard values
#define WS2812_T0H_NS   400    // 0 bit high time (0.4µs)
#define WS2812_T0L_NS   850    // 0 bit low time (0.85µs)
#define WS2812_T1H_NS   800    // 1 bit high time (0.8µs)
#define WS2812_T1L_NS   450    // 1 bit low time (0.45µs)
#define WS2812_RESET_NS 50000  // Reset time (50µs)

// RMT resolution and frequency - Use 40MHz for precise timing
#define RMT_RESOLUTION_HZ 40000000 // 40MHz resolution, 1 tick = 25ns

// Static variables
static bool rgb_led_initialized = false;
static int led_count = 1;
static rmt_channel_handle_t rmt_channel = NULL;
static rmt_encoder_handle_t ws2812_encoder = NULL;

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

/**
 * @brief WS2812 encoder structure
 */
typedef struct {
    rmt_encoder_t base;
    rmt_encoder_t *bytes_encoder;
    rmt_encoder_t *copy_encoder;
    int state;
    rmt_symbol_word_t reset_code;
} rmt_ws2812_encoder_t;

/**
 * @brief Encode WS2812 data
 */
static size_t rmt_encode_ws2812(rmt_encoder_t *encoder, rmt_channel_handle_t channel,
                                const void *primary_data, size_t data_size, rmt_encode_state_t *ret_state)
{
    rmt_ws2812_encoder_t *ws2812_encoder = __containerof(encoder, rmt_ws2812_encoder_t, base);
    rmt_encoder_handle_t bytes_encoder = ws2812_encoder->bytes_encoder;
    rmt_encoder_handle_t copy_encoder = ws2812_encoder->copy_encoder;
    rmt_encode_state_t session_state = 0;
    rmt_encode_state_t state = 0;
    size_t encoded_symbols = 0;

    switch (ws2812_encoder->state) {
    case 0: // send RGB data
        encoded_symbols += bytes_encoder->encode(bytes_encoder, channel, primary_data, data_size, &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            ws2812_encoder->state = 1; // switch to next state when current encoding session finished
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out; // yield if there's no free space for encoding artifacts
        }
    // fall-through
    case 1: // send reset code
        encoded_symbols += copy_encoder->encode(copy_encoder, channel, &ws2812_encoder->reset_code,
                                                sizeof(ws2812_encoder->reset_code), &session_state);
        if (session_state & RMT_ENCODING_COMPLETE) {
            ws2812_encoder->state = 0; // back to the initial encoding session
            state |= RMT_ENCODING_COMPLETE;
        }
        if (session_state & RMT_ENCODING_MEM_FULL) {
            state |= RMT_ENCODING_MEM_FULL;
            goto out; // yield if there's no free space for encoding artifacts
        }
    }
out:
    *ret_state = state;
    return encoded_symbols;
}

/**
 * @brief Delete WS2812 encoder
 */
static esp_err_t rmt_del_ws2812_encoder(rmt_encoder_t *encoder)
{
    rmt_ws2812_encoder_t *ws2812_encoder = __containerof(encoder, rmt_ws2812_encoder_t, base);
    rmt_del_encoder(ws2812_encoder->bytes_encoder);
    rmt_del_encoder(ws2812_encoder->copy_encoder);
    free(ws2812_encoder);
    return ESP_OK;
}

/**
 * @brief Reset WS2812 encoder
 */
static esp_err_t rmt_ws2812_encoder_reset(rmt_encoder_t *encoder)
{
    rmt_ws2812_encoder_t *ws2812_encoder = __containerof(encoder, rmt_ws2812_encoder_t, base);
    rmt_encoder_reset(ws2812_encoder->bytes_encoder);
    rmt_encoder_reset(ws2812_encoder->copy_encoder);
    ws2812_encoder->state = 0;
    return ESP_OK;
}

/**
 * @brief Create WS2812 encoder
 */
static esp_err_t rmt_new_ws2812_encoder(rmt_encoder_handle_t *ret_encoder)
{
    esp_err_t ret = ESP_OK;
    rmt_ws2812_encoder_t *ws2812_encoder = NULL;

    ws2812_encoder = calloc(1, sizeof(rmt_ws2812_encoder_t));
    if (!ws2812_encoder) {
        return ESP_ERR_NO_MEM;
    }

    ws2812_encoder->base.encode = rmt_encode_ws2812;
    ws2812_encoder->base.del = rmt_del_ws2812_encoder;
    ws2812_encoder->base.reset = rmt_ws2812_encoder_reset;

    // Calculate timing values in RMT ticks - use proper calculation to avoid truncation
    uint32_t t0h_ticks = (uint32_t)((uint64_t)WS2812_T0H_NS * RMT_RESOLUTION_HZ / 1000000000ULL);
    uint32_t t0l_ticks = (uint32_t)((uint64_t)WS2812_T0L_NS * RMT_RESOLUTION_HZ / 1000000000ULL);
    uint32_t t1h_ticks = (uint32_t)((uint64_t)WS2812_T1H_NS * RMT_RESOLUTION_HZ / 1000000000ULL);
    uint32_t t1l_ticks = (uint32_t)((uint64_t)WS2812_T1L_NS * RMT_RESOLUTION_HZ / 1000000000ULL);

    ESP_LOGI(TAG, "WS2812 timing: T0H=%ld, T0L=%ld, T1H=%ld, T1L=%ld ticks",
             t0h_ticks, t0l_ticks, t1h_ticks, t1l_ticks);

    // Validate timing values
    if (t0h_ticks == 0 || t0l_ticks == 0 || t1h_ticks == 0 || t1l_ticks == 0) {
        ESP_LOGE(TAG, "Invalid timing values calculated! Check RMT resolution.");
        return ESP_ERR_INVALID_ARG;
    }

    // Different bit patterns for 0 and 1
    rmt_bytes_encoder_config_t bytes_encoder_config = {
        .bit0 = {
            .level0 = 1,
            .duration0 = t0h_ticks, // T0H
            .level1 = 0,
            .duration1 = t0l_ticks, // T0L
        },
        .bit1 = {
            .level0 = 1,
            .duration0 = t1h_ticks, // T1H
            .level1 = 0,
            .duration1 = t1l_ticks, // T1L
        },
        .flags.msb_first = 1 // WS2812 transfers MSB first
    };

    ret = rmt_new_bytes_encoder(&bytes_encoder_config, &ws2812_encoder->bytes_encoder);
    if (ret != ESP_OK) {
        goto err;
    }

    rmt_copy_encoder_config_t copy_encoder_config = {};
    ret = rmt_new_copy_encoder(&copy_encoder_config, &ws2812_encoder->copy_encoder);
    if (ret != ESP_OK) {
        goto err;
    }

    uint32_t reset_ticks = (uint32_t)((uint64_t)WS2812_RESET_NS * RMT_RESOLUTION_HZ / 1000000000ULL);
    ESP_LOGI(TAG, "WS2812 reset timing: %ld ticks (%d ns)", reset_ticks, WS2812_RESET_NS);

    if (reset_ticks == 0) {
        ESP_LOGE(TAG, "Invalid reset timing calculated!");
        ret = ESP_ERR_INVALID_ARG;
        goto err;
    }

    ws2812_encoder->reset_code = (rmt_symbol_word_t) {
        .level0 = 0, .duration0 = reset_ticks,
        .level1 = 0, .duration1 = reset_ticks,
    };

    *ret_encoder = &ws2812_encoder->base;
    return ESP_OK;

err:
    if (ws2812_encoder) {
        if (ws2812_encoder->bytes_encoder) {
            rmt_del_encoder(ws2812_encoder->bytes_encoder);
        }
        if (ws2812_encoder->copy_encoder) {
            rmt_del_encoder(ws2812_encoder->copy_encoder);
        }
        free(ws2812_encoder);
    }
    return ret;
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

    if (rgb_led_initialized) {
        ESP_LOGW(TAG, "RGB LED already initialized");
        return ESP_OK;
    }

    esp_err_t ret;

    // Configure GPIO first
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << config->gpio_pin),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO%d: %s", config->gpio_pin, esp_err_to_name(ret));
        return ret;
    }

    // Set GPIO to low initially
    gpio_set_level(config->gpio_pin, 0);
    ESP_LOGI(TAG, "GPIO%d configured as output", config->gpio_pin);

    // Create RMT TX channel
    rmt_tx_channel_config_t tx_chan_config = {
        .clk_src = RMT_CLK_SRC_DEFAULT,
        .gpio_num = config->gpio_pin,
        .mem_block_symbols = 64, // increase the block size can make the LED less flickering
        .resolution_hz = RMT_RESOLUTION_HZ,
        .trans_queue_depth = 4, // set the number of transactions that can be pending in the background
    };

    ret = rmt_new_tx_channel(&tx_chan_config, &rmt_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create RMT TX channel: %s", esp_err_to_name(ret));
        return ret;
    }

    // Create WS2812 encoder
    ret = rmt_new_ws2812_encoder(&ws2812_encoder);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create WS2812 encoder: %s", esp_err_to_name(ret));
        rmt_del_channel(rmt_channel);
        rmt_channel = NULL;
        return ret;
    }

    // Enable RMT TX channel
    ret = rmt_enable(rmt_channel);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to enable RMT channel: %s", esp_err_to_name(ret));
        rmt_del_encoder(ws2812_encoder);
        rmt_del_channel(rmt_channel);
        rmt_channel = NULL;
        ws2812_encoder = NULL;
        return ret;
    }

    led_count = config->led_count;
    rgb_led_initialized = true;

    ESP_LOGI(TAG, "RGB LED initialized successfully with RMT channel");

    // Turn off LED initially
    rgb_led_off();

    return ESP_OK;
}

esp_err_t rgb_led_set_color(uint8_t red, uint8_t green, uint8_t blue) {
    if (!rgb_led_initialized) {
        ESP_LOGE(TAG, "RGB LED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    if (!rmt_channel || !ws2812_encoder) {
        ESP_LOGE(TAG, "RMT channel or encoder not available");
        return ESP_ERR_INVALID_STATE;
    }

    // Try RGB order instead of GRB (some WS2812 variants use different orders)
    uint8_t led_data[3] = {red, green, blue};

    // Prepare transmission configuration
    rmt_transmit_config_t tx_config = {
        .loop_count = 0, // no transfer loop
    };

    // Transmit the LED data
    esp_err_t ret = rmt_transmit(rmt_channel, ws2812_encoder, led_data, sizeof(led_data), &tx_config);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to transmit LED data: %s", esp_err_to_name(ret));
        return ret;
    }

    // Wait for transmission to complete
    ret = rmt_tx_wait_all_done(rmt_channel, portMAX_DELAY);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to wait for transmission completion: %s", esp_err_to_name(ret));
        return ret;
    }

    ESP_LOGI(TAG, "Set RGB LED color: R=%d, G=%d, B=%d (RGB data: 0x%02X%02X%02X)",
             red, green, blue, led_data[0], led_data[1], led_data[2]);

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

esp_err_t rgb_led_deinit(void) {
    if (!rgb_led_initialized) {
        return ESP_OK;
    }

    // Turn off LED first
    rgb_led_off();

    // Disable and cleanup RMT channel
    if (rmt_channel) {
        rmt_disable(rmt_channel);
        rmt_del_channel(rmt_channel);
        rmt_channel = NULL;
    }

    // Cleanup encoder
    if (ws2812_encoder) {
        rmt_del_encoder(ws2812_encoder);
        ws2812_encoder = NULL;
    }

    rgb_led_initialized = false;

    ESP_LOGI(TAG, "RGB LED deinitialized");

    return ESP_OK;
}

esp_err_t rgb_led_test(void) {
    if (!rgb_led_initialized) {
        ESP_LOGE(TAG, "RGB LED not initialized");
        return ESP_ERR_INVALID_STATE;
    }

    ESP_LOGI(TAG, "Starting RGB LED test sequence...");

    // Test with maximum brightness colors
    ESP_LOGI(TAG, "Test 1: Red (255, 0, 0)");
    rgb_led_set_color(255, 0, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "Test 2: Green (0, 255, 0)");
    rgb_led_set_color(0, 255, 0);
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "Test 3: Blue (0, 0, 255)");
    rgb_led_set_color(0, 0, 255);
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "Test 4: White (255, 255, 255)");
    rgb_led_set_color(255, 255, 255);
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "Test 5: Dim white (32, 32, 32)");
    rgb_led_set_color(32, 32, 32);
    vTaskDelay(pdMS_TO_TICKS(1000));

    ESP_LOGI(TAG, "Test 6: Off");
    rgb_led_off();
    vTaskDelay(pdMS_TO_TICKS(500));

    ESP_LOGI(TAG, "RGB LED test sequence completed");

    return ESP_OK;
}

esp_err_t rgb_led_gpio_test(void) {
    ESP_LOGI(TAG, "Testing GPIO%d by toggling...", RGB_LED_PIN);

    // Configure GPIO as output
    gpio_config_t io_conf = {
        .pin_bit_mask = (1ULL << RGB_LED_PIN),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE,
    };
    esp_err_t ret = gpio_config(&io_conf);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to configure GPIO%d: %s", RGB_LED_PIN, esp_err_to_name(ret));
        return ret;
    }

    // Toggle GPIO 10 times
    for (int i = 0; i < 10; i++) {
        gpio_set_level(RGB_LED_PIN, 1);
        ESP_LOGI(TAG, "GPIO%d HIGH", RGB_LED_PIN);
        vTaskDelay(pdMS_TO_TICKS(500));

        gpio_set_level(RGB_LED_PIN, 0);
        ESP_LOGI(TAG, "GPIO%d LOW", RGB_LED_PIN);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    ESP_LOGI(TAG, "GPIO test completed");
    return ESP_OK;
}
