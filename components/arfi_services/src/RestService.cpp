#include "arfi/services/RestService.hpp"
#include "arfi/services/WifiService.hpp"

#include "esp_err.h"
#include "esp_http_client.h"
#include "esp_log.h"

#include <algorithm>

namespace arfi {

static const char* TAG = "rest_service";

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
        response.error_name = esp_err_to_name(response.error);
        return response.error;
    }

    response.error = wifi_->connect();
    if (response.error != ESP_OK) {
        response.error_name = esp_err_to_name(response.error);
        return response.error;
    }

    HttpAccumulator accumulator;
    accumulator.body = &response.body;
    accumulator.max_body_bytes = max_body_bytes;

    esp_http_client_config_t config = {};
    config.url = url;
    config.method = HTTP_METHOD_GET;
    config.timeout_ms = 12000;
    config.user_agent = "arfiOS/1.0";
    config.disable_auto_redirect = false;
    config.max_redirection_count = 5;
    config.event_handler = httpEventHandler;
    config.user_data = &accumulator;
    config.buffer_size = 1024;
    config.buffer_size_tx = 1024;
    config.keep_alive_enable = false;

    esp_http_client_handle_t client = esp_http_client_init(&config);
    if (client == nullptr) {
        response.error = ESP_ERR_NO_MEM;
        response.error_name = esp_err_to_name(response.error);
        return response.error;
    }

    (void)esp_http_client_set_header(client, "Accept", "application/json, text/plain, */*");
    (void)esp_http_client_set_header(client, "Connection", "close");
    (void)esp_http_client_set_header(client, "Cache-Control", "no-cache");

    response.error = esp_http_client_perform(client);
    response.status_code = esp_http_client_get_status_code(client);
    response.content_length = esp_http_client_get_content_length(client);
    response.truncated = accumulator.truncated;
    response.error_name = esp_err_to_name(response.error);
    response.transport_errno = esp_http_client_get_errno(client);

    if (response.error != ESP_OK) {
        ESP_LOGW(
            TAG,
            "GET failed url=%s err=%s(0x%x) http=%d errno=%d",
            url,
            response.error_name.c_str(),
            static_cast<unsigned>(response.error),
            response.status_code,
            response.transport_errno);
    }

    esp_http_client_cleanup(client);
    return response.error;
}

} // namespace arfi
