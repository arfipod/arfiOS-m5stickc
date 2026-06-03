#include "arfi/services/SettingsService.hpp"

#include "esp_log.h"

#include <vector>

namespace arfi {

static const char* TAG = "settings";

esp_err_t SettingsService::begin() {
    if (initialized_) {
        return ESP_OK;
    }

    esp_err_t err = nvs_open("arfi", NVS_READWRITE, &handle_);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "nvs_open failed: %s", esp_err_to_name(err));
        return err;
    }

    initialized_ = true;
    return ESP_OK;
}

int32_t SettingsService::getInt(const char* key, int32_t fallback) {
    int32_t value = fallback;
    if (!initialized_ || nvs_get_i32(handle_, key, &value) != ESP_OK) {
        return fallback;
    }
    return value;
}

void SettingsService::setInt(const char* key, int32_t value) {
    if (!initialized_) {
        return;
    }
    if (nvs_set_i32(handle_, key, value) == ESP_OK) {
        nvs_commit(handle_);
    }
}

bool SettingsService::getBool(const char* key, bool fallback) {
    return getInt(key, fallback ? 1 : 0) != 0;
}

void SettingsService::setBool(const char* key, bool value) { setInt(key, value ? 1 : 0); }

std::string SettingsService::getString(const char* key, const char* fallback) {
    if (!initialized_) {
        return fallback;
    }

    size_t required = 0;
    if (nvs_get_str(handle_, key, nullptr, &required) != ESP_OK || required == 0) {
        return fallback;
    }

    std::vector<char> buffer(required);
    if (nvs_get_str(handle_, key, buffer.data(), &required) != ESP_OK) {
        return fallback;
    }

    return std::string(buffer.data());
}

void SettingsService::setString(const char* key, const char* value) {
    if (!initialized_) {
        return;
    }
    if (nvs_set_str(handle_, key, value) == ESP_OK) {
        nvs_commit(handle_);
    }
}

} // namespace arfi
