idf_component_register(
    SRCS
        "src/M5StickCPlusDisplay.cpp"
        "src/M5StickCPlusImu.cpp"
        "src/M5StickCPlusIr.cpp"
        "src/M5StickCPlusPower.cpp"
    INCLUDE_DIRS "include"
    REQUIRES
        driver
        esp_lcd
        esp_timer
)
