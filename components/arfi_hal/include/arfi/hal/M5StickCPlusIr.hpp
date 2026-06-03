#pragma once

#include "arfi/hal/BoardConfig.hpp"

#include "esp_err.h"

#include <cstdint>

namespace arfi {

class M5StickCPlusIr {
public:
    esp_err_t begin(const BoardConfig& board);
    esp_err_t emitCarrier(uint32_t carrier_hz);
    esp_err_t stop();

    bool active() const { return active_; }
    uint32_t carrierHz() const { return carrier_hz_; }

private:
    bool initialized_ = false;
    bool active_ = false;
    uint32_t carrier_hz_ = 0;
};

} // namespace arfi
