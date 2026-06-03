#include "arfi/runtime/System.hpp"

#include "esp_log.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char* TAG = "arfi_main";

extern "C" void app_main(void) {
    ESP_LOGI(TAG, "Starting arfiOS");

    arfi::System system;
    esp_err_t err = system.begin();
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "System initialization failed: %s", esp_err_to_name(err));
        return;
    }

    while (true) {
        system.tick();
        vTaskDelay(pdMS_TO_TICKS(1));
    }
}
