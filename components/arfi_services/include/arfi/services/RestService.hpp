#pragma once

#include "esp_err.h"

#include <cstddef>
#include <cstdint>
#include <string>

namespace arfi {

class WifiService;

struct RestResponse {
    esp_err_t error = ESP_ERR_INVALID_STATE;
    std::string error_name;
    int transport_errno = 0;
    int status_code = 0;
    int64_t content_length = -1;
    bool truncated = false;
    std::string body;
};

class RestService {
public:
    esp_err_t begin(WifiService& wifi);
    esp_err_t get(const char* url, RestResponse& response, size_t max_body_bytes = 512);

    bool available() const { return wifi_ != nullptr; }

private:
    WifiService* wifi_ = nullptr;
};

} // namespace arfi
