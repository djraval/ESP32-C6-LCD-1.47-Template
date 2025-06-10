idf_component_register(SRCS "src/ui_manager.c"
                            "src/lvgl_driver.c"
                       INCLUDE_DIRS "include"
                       REQUIRES lvgl board_hal wifi_hal time_hal)