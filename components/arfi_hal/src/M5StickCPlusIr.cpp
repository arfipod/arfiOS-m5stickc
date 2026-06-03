#include "arfi/hal/M5StickCPlusIr.hpp"
#include "arfi/hal/M5StickCPlusPins.hpp"

#include "driver/ledc.h"
#include "esp_check.h"
#include "esp_log.h"

#include <algorithm>

namespace arfi {

static const char* TAG = "m5_ir";

namespace {
constexpr ledc_mode_t kSpeedMode = LEDC_LOW_SPEED_MODE;
constexpr ledc_timer_t kTimer = LEDC_TIMER_0;
constexpr ledc_channel_t kChannel = LEDC_CHANNEL_0;
constexpr ledc_timer_bit_t kDutyResolution = LEDC_TIMER_8_BIT;
constexpr uint32_t kDefaultCarrierHz = 38000;
constexpr uint32_t kMinCarrierHz = 30000;
constexpr uint32_t kMaxCarrierHz = 60000;
constexpr uint32_t kHalfDuty = 128;
} // namespace

esp_err_t M5StickCPlusIr::begin(const BoardConfig& board) {
    if (initialized_) {
        return ESP_OK;
    }

    if (board.type != BoardType::M5StickCPlus) {
        return ESP_ERR_NOT_SUPPORTED;
    }

    ledc_timer_config_t timer = {};
    timer.speed_mode = kSpeedMode;
    timer.timer_num = kTimer;
    timer.duty_resolution = kDutyResolution;
    timer.freq_hz = kDefaultCarrierHz;
    timer.clk_cfg = LEDC_AUTO_CLK;
    ESP_RETURN_ON_ERROR(ledc_timer_config(&timer), TAG, "LEDC timer config failed");

    ledc_channel_config_t channel = {};
    channel.gpio_num = m5stickcplus::kIrLed;
    channel.speed_mode = kSpeedMode;
    channel.channel = kChannel;
    channel.intr_type = LEDC_INTR_DISABLE;
    channel.timer_sel = kTimer;
    channel.duty = 0;
    channel.hpoint = 0;
    ESP_RETURN_ON_ERROR(ledc_channel_config(&channel), TAG, "LEDC channel config failed");

    initialized_ = true;
    active_ = false;
    carrier_hz_ = 0;
    ESP_LOGI(TAG, "IR LED initialized");
    return ESP_OK;
}

esp_err_t M5StickCPlusIr::emitCarrier(uint32_t carrier_hz) {
    if (!initialized_) {
        return ESP_ERR_INVALID_STATE;
    }

    carrier_hz = std::clamp(carrier_hz, kMinCarrierHz, kMaxCarrierHz);
    ESP_RETURN_ON_ERROR(ledc_set_freq(kSpeedMode, kTimer, carrier_hz), TAG, "IR carrier frequency setup failed");
    ESP_RETURN_ON_ERROR(ledc_set_duty(kSpeedMode, kChannel, kHalfDuty), TAG, "IR duty setup failed");
    ESP_RETURN_ON_ERROR(ledc_update_duty(kSpeedMode, kChannel), TAG, "IR duty update failed");

    active_ = true;
    carrier_hz_ = carrier_hz;
    return ESP_OK;
}

esp_err_t M5StickCPlusIr::stop() {
    if (!initialized_) {
        return ESP_OK;
    }

    active_ = false;
    carrier_hz_ = 0;
    return ledc_stop(kSpeedMode, kChannel, 0);
}

} // namespace arfi
