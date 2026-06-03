#include "arfi/services/WifiService.hpp"

#include "esp_check.h"
#include "esp_log.h"
#include "esp_wifi.h"

#include <cstdio>
#include <cstring>

#ifndef ARFI_WIFI_SSID
#define ARFI_WIFI_SSID ""
#endif

#ifndef ARFI_WIFI_PASSWORD
#define ARFI_WIFI_PASSWORD ""
#endif

namespace arfi {

static const char* TAG = "wifi_service";

esp_err_t WifiService::begin() {
    if (initialized_) {
        return ESP_OK;
    }

    ssid_ = ARFI_WIFI_SSID;
    password_ = ARFI_WIFI_PASSWORD;

    events_ = xEventGroupCreate();
    if (events_ == nullptr) {
        last_error_ = ESP_ERR_NO_MEM;
        return last_error_;
    }

    esp_err_t err = esp_netif_init();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        last_error_ = err;
        return err;
    }

    err = esp_event_loop_create_default();
    if (err != ESP_OK && err != ESP_ERR_INVALID_STATE) {
        last_error_ = err;
        return err;
    }

    netif_ = esp_netif_create_default_wifi_sta();
    if (netif_ == nullptr) {
        last_error_ = ESP_ERR_NO_MEM;
        return last_error_;
    }

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_RETURN_ON_ERROR(esp_wifi_init(&cfg), TAG, "wifi init failed");
    ESP_RETURN_ON_ERROR(
        esp_event_handler_instance_register(WIFI_EVENT, ESP_EVENT_ANY_ID, &WifiService::eventHandler_, this, nullptr),
        TAG,
        "wifi event handler register failed");
    ESP_RETURN_ON_ERROR(
        esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &WifiService::eventHandler_, this, nullptr),
        TAG,
        "ip event handler register failed");
    ESP_RETURN_ON_ERROR(esp_wifi_set_mode(WIFI_MODE_STA), TAG, "wifi mode setup failed");

    initialized_ = true;
    last_error_ = ESP_OK;
    ESP_LOGI(TAG, "Wi-Fi initialized");
    return ESP_OK;
}

esp_err_t WifiService::connect(uint32_t timeout_ms) {
    if (!initialized_) {
        last_error_ = ESP_ERR_INVALID_STATE;
        return last_error_;
    }

    if (connected_) {
        return ESP_OK;
    }

    if (ssid_.empty()) {
        last_error_ = ESP_ERR_INVALID_ARG;
        return last_error_;
    }

    wifi_config_t wifi_config = {};
    std::snprintf(reinterpret_cast<char*>(wifi_config.sta.ssid), sizeof(wifi_config.sta.ssid), "%s", ssid_.c_str());
    std::snprintf(
        reinterpret_cast<char*>(wifi_config.sta.password),
        sizeof(wifi_config.sta.password),
        "%s",
        password_.c_str());
    wifi_config.sta.threshold.authmode = password_.empty() ? WIFI_AUTH_OPEN : WIFI_AUTH_WPA2_PSK;

    xEventGroupClearBits(events_, kConnectedBit | kFailBit);
    retry_count_ = 0;
    connected_ = false;
    connecting_ = true;

    esp_err_t err = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (err != ESP_OK) {
        connecting_ = false;
        last_error_ = err;
        return err;
    }

    if (!started_) {
        err = esp_wifi_start();
        if (err != ESP_OK) {
            connecting_ = false;
            last_error_ = err;
            return err;
        }
        started_ = true;
    } else {
        (void)esp_wifi_disconnect();
    }

    err = esp_wifi_connect();
    if (err != ESP_OK && err != ESP_ERR_WIFI_CONN) {
        connecting_ = false;
        last_error_ = err;
        return err;
    }

    const EventBits_t bits = xEventGroupWaitBits(
        events_,
        kConnectedBit | kFailBit,
        pdFALSE,
        pdFALSE,
        pdMS_TO_TICKS(timeout_ms));

    connecting_ = false;
    if (bits & kConnectedBit) {
        last_error_ = ESP_OK;
        return ESP_OK;
    }

    last_error_ = (bits & kFailBit) ? ESP_FAIL : ESP_ERR_TIMEOUT;
    return last_error_;
}

void WifiService::eventHandler_(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    auto* self = static_cast<WifiService*>(arg);
    if (self != nullptr) {
        self->handleEvent_(event_base, event_id, event_data);
    }
}

void WifiService::handleEvent_(esp_event_base_t event_base, int32_t event_id, void*) {
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        connected_ = false;
        if (connecting_ && retry_count_ < kMaxRetries) {
            ++retry_count_;
            (void)esp_wifi_connect();
            return;
        }
        xEventGroupSetBits(events_, kFailBit);
        return;
    }

    if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        connected_ = true;
        connecting_ = false;
        retry_count_ = 0;
        xEventGroupSetBits(events_, kConnectedBit);
    }
}

} // namespace arfi
