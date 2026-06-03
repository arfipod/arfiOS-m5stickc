#pragma once

#include "arfi/hal/BoardConfig.hpp"
#include "arfi/hal/M5StickCPlusIr.hpp"

#include "esp_err.h"

#include <cstdint>

namespace arfi {

class IrService {
public:
    esp_err_t begin(const BoardConfig& board);
    void update(uint32_t now_ms);
    esp_err_t emitBurst(uint32_t carrier_hz, uint16_t duration_ms);
    void stop();

    bool available() const { return available_; }
    bool active() const { return active_; }
    uint32_t carrierHz() const { return carrier_hz_; }
    esp_err_t lastError() const { return last_error_; }

private:
    BoardConfig board_;
    M5StickCPlusIr m5_ir_;
    bool available_ = false;
    bool active_ = false;
    uint32_t carrier_hz_ = 0;
    uint32_t burst_end_ms_ = 0;
    esp_err_t last_error_ = ESP_ERR_INVALID_STATE;
};

} // namespace arfi
