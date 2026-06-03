#include "arfi/services/RestService.hpp"
#include "arfi/services/WifiService.hpp"

#include "esp_http_client.h"

#include <algorithm>

namespace arfi {

namespace {
struct HttpAccumulator {
    std::string* body = nullptr;
    size_t max_body_bytes = 0;
    bool truncated = false;
};

esp_err_t httpEventHandler(esp_http_client_event_t* event) {
    if (event->event_id != HTTP_EVENT_ON_DATA || event->data == nullptr || event->data_len <= 0) {
        return ESP_OK;
    }

    auto* accumulator = static_cast<HttpAccumulator*>(event->user_data);
    if (accumulator == nullptr || accumulator->body == nullptr) {
        return ESP_OK;
    }

    const size_t available = accumulator->max_body_bytes - accumulator->body->size();
    const size_t copy_len = std::min<size_t>(available, static_cast<size_t>(event->data_len));
    if (copy_len > 0) {
        accumulator->body->append(static_cast<const char*>(event->data), copy_len);
    }
    if (copy_len < static_cast<size_t>(event->data_len)) {
        accumulator->truncated = true;
    }

    return ESP_OK;
}
} // namespace

esp_err_t RestService::begin(WifiService& wifi) {
    wifi_ = &wifi;
    return ESP_OK;
}

esp_err_t RestService::get(const char* url, RestResponse& response, size_t max_body_bytes) {
    response = RestResponse{};

    if (wifi_ == nullptr || url == nullptr || url[0] == '\0') {
        response.error = ESP_ERR_INVALID_ARG;
        return response.error;
    }

    response.error = wifi_->connect();
    if (response.error != ESP_OK) {
        return response.error;
    }

    HttpAccumulator accumulator;
    accumulator.body = &response.body;
    accumulator.max_body_bytes = max_body_bytes;

    esp_http_client_config_t config = {};
    config.url = url;
    config.timeout_ms = 6000;
    config.event_handler = httpEventHandler;
    config.user_data = &accumulator;
    config.buffer_size = 512;
    config.buffer_size_tx = 512;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == nullptr) {
        response.error = ESP_ERR_NO_MEM;
        return response.error;
    }

    response.error = esp_http_client_perform(client);
    response.status_code = esp_http_client_get_status_code(client);
    response.content_length = esp_http_client_get_content_length(client);
    response.truncated = accumulator.truncated;

    esp_http_client_cleanup(client);
    return response.error;
}

} // namespace arfi
