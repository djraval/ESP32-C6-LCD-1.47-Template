idf_component_register(SRCS "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES board_hal app_ui wifi_hal time_hal)
