#pragma once

#include "esp_err.h"
#include "nvs.h"

#include <cstdint>
#include <string>

namespace arfi {

class SettingsService {
public:
    esp_err_t begin();

    int32_t getInt(const char* key, int32_t fallback);
    void setInt(const char* key, int32_t value);

    bool getBool(const char* key, bool fallback);
    void setBool(const char* key, bool value);

    std::string getString(const char* key, const char* fallback);
    void setString(const char* key, const char* value);

private:
    nvs_handle_t handle_ = 0;
    bool initialized_ = false;
};

} // namespace arfi
