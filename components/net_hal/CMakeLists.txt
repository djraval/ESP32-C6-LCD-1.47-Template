idf_component_register(SRCS "src/net_hal.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_wifi esp_netif esp_event nvs_flash mqtt)
