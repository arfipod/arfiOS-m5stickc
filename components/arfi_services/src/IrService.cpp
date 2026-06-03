#include "arfi/services/IrService.hpp"

#include "esp_timer.h"

#include <algorithm>

namespace arfi {

esp_err_t IrService::begin(const BoardConfig& board) {
    board_ = board;
    available_ = false;
    active_ = false;

    if (!board.has_ir) {
        last_error_ = ESP_ERR_NOT_SUPPORTED;
        return last_error_;
    }

    if (board.type == BoardType::M5StickCPlus) {
        last_error_ = m5_ir_.begin(board);
        available_ = last_error_ == ESP_OK;
        return last_error_;
    }

    last_error_ = ESP_ERR_NOT_SUPPORTED;
    return last_error_;
}

void IrService::update(uint32_t now_ms) {
    if (!active_) {
        return;
    }

    if (static_cast<int32_t>(now_ms - burst_end_ms_) >= 0) {
        stop();
    }
}

esp_err_t IrService::emitBurst(uint32_t carrier_hz, uint16_t duration_ms) {
    if (!available_) {
        return last_error_;
    }

    if (duration_ms == 0) {
        stop();
        return ESP_OK;
    }

    duration_ms = std::min<uint16_t>(duration_ms, 500);

    if (board_.type == BoardType::M5StickCPlus) {
        last_error_ = m5_ir_.emitCarrier(carrier_hz);
        if (last_error_ == ESP_OK) {
            active_ = true;
            carrier_hz_ = m5_ir_.carrierHz();
            const uint32_t now_ms = static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
            burst_end_ms_ = now_ms + duration_ms;
        }
        return last_error_;
    }

    last_error_ = ESP_ERR_NOT_SUPPORTED;
    return last_error_;
}

void IrService::stop() {
    if (board_.type == BoardType::M5StickCPlus) {
        last_error_ = m5_ir_.stop();
    }
    active_ = false;
    carrier_hz_ = 0;
}

} // namespace arfi
