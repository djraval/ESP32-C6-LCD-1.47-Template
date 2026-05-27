if(CONFIG_APP_TIMER_ENABLED)
    set(srcs "src/app_timer.c")
else()
    set(srcs)
endif()

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_timer
)
