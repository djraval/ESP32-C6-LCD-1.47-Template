if(CONFIG_APP_HTTP_ENABLED)
    set(srcs "src/app_http.c")
else()
    set(srcs)
endif()

idf_component_register(
    SRCS ${srcs}
    INCLUDE_DIRS "include"
    PRIV_REQUIRES esp_http_client
)
