if(CONFIG_APP_BUTTON_ENABLED)
    set(srcs "src/app_button.c")
else()
    set(srcs)
endif()

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_driver_gpio esp_timer
)
