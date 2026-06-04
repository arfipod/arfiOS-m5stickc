#pragma once

#include "esp_err.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/event_groups.h"

#include <cstdint>
#include <string>

namespace arfi {

class WifiService {
public:
    esp_err_t begin();
    esp_err_t connect(uint32_t timeout_ms = 10000);

    bool initialized() const { return initialized_; }
    bool connected() const { return connected_; }
    bool hasCredentials() const { return !ssid_.empty(); }
    const char* ssid() const { return ssid_.c_str(); }
    esp_err_t lastError() const { return last_error_; }
    int8_t rssiDbm() const;
    uint8_t signalLevel() const;

private:
    static void eventHandler_(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data);
    void handleEvent_(esp_event_base_t event_base, int32_t event_id, void* event_data);

    static constexpr EventBits_t kConnectedBit = BIT0;
    static constexpr EventBits_t kFailBit = BIT1;
    static constexpr uint8_t kMaxRetries = 5;

    EventGroupHandle_t events_ = nullptr;
    esp_netif_t* netif_ = nullptr;
    bool initialized_ = false;
    bool started_ = false;
    bool connected_ = false;
    bool connecting_ = false;
    uint8_t retry_count_ = 0;
    std::string ssid_;
    std::string password_;
    esp_err_t last_error_ = ESP_ERR_INVALID_STATE;
};

} // namespace arfi
