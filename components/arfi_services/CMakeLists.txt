idf_component_register(
    SRCS
        "src/DisplayService.cpp"
        "src/InputService.cpp"
        "src/PowerService.cpp"
        "src/SettingsService.cpp"
    INCLUDE_DIRS "include"
    REQUIRES
        arfi_core
        arfi_hal
        arfi_ui
        driver
        nvs_flash
        esp_timer
)
