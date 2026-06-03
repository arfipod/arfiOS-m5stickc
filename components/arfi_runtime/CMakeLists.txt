idf_component_register(
    SRCS
        "src/System.cpp"
    INCLUDE_DIRS "include"
    REQUIRES
        arfi_core
        arfi_hal
        arfi_services
        arfi_ui
        arfi_apps
        esp_timer
        nvs_flash
)
