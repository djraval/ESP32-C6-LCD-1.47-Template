/*
 * LED Hardware Abstraction Layer Implementation
 * 
 * RGB LED control for ESP32-C6-LCD-1.47
 */

#include "led_hal.h"
#include "board_config.h"
#include "led_strip.h"
#include "esp_log.h"

static const char *TAG = "LED_HAL";

// Hardware handle
static led_strip_handle_t rgb_led_strip = NULL;

esp_err_t led_hal_init(void)
{
    ESP_LOGI(TAG, "Initializing RGB LED");
    
    esp_err_t ret = led_strip_new_rmt_device(
        &(led_strip_config_t){
            .strip_gpio_num = RGB_LED_PIN,
            .max_leds = 1,
            .led_pixel_format = LED_PIXEL_FORMAT_GRB,
            .led_model = LED_MODEL_WS2812
        },
        &(led_strip_rmt_config_t){
            .clk_src = RMT_CLK_SRC_DEFAULT,
            .resolution_hz = 10000000
        },
        &rgb_led_strip);
    
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to create LED strip: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Turn off initially
    ret = led_strip_clear(rgb_led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear LED strip: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ESP_LOGI(TAG, "RGB LED initialized successfully");
    return ESP_OK;
}

esp_err_t led_hal_set_color(uint8_t red, uint8_t green, uint8_t blue)
{
    if (rgb_led_strip == NULL) {
        ESP_LOGE(TAG, "LED strip not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = led_strip_set_pixel(rgb_led_strip, 0, red, green, blue);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to set LED pixel: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = led_strip_refresh(rgb_led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to refresh LED strip: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}

esp_err_t led_hal_clear(void)
{
    if (rgb_led_strip == NULL) {
        ESP_LOGE(TAG, "LED strip not initialized");
        return ESP_ERR_INVALID_STATE;
    }
    
    esp_err_t ret = led_strip_clear(rgb_led_strip);
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "Failed to clear LED strip: %s", esp_err_to_name(ret));
        return ret;
    }
    
    return ESP_OK;
}