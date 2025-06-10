idf_component_register(SRCS "src/display_hal.c"
                            "src/led_hal.c"
                       INCLUDE_DIRS "include"
                       REQUIRES esp_driver_gpio esp_driver_spi esp_driver_ledc esp_lcd led_strip)