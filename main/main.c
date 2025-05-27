#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_log.h"

static const char *TAG = "ESP32C6_PROJECT";

void app_main(void)
{
    ESP_LOGI(TAG, "Hello World from ESP32-C6!");
    
    while (1) {
        ESP_LOGI(TAG, "ESP32-C6 is running...");
        vTaskDelay(pdMS_TO_TICKS(5000));
    }
}
