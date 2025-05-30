idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_driver_gpio esp_driver_spi esp_driver_ledc esp_driver_rmt esp_lcd lvgl
                    PRIV_REQUIRES lcd_st7789 rgb_led)
