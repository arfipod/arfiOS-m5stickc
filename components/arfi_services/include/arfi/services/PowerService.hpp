#pragma once

#include "arfi/hal/BoardConfig.hpp"
#include "arfi/hal/M5StickCPlusPower.hpp"

#include "esp_err.h"

#include <cstdint>

namespace arfi {

class PowerService {
public:
    esp_err_t begin(const BoardConfig& board);
    esp_err_t setBacklightPercent(uint8_t percent);
    uint8_t backlightPercent() const { return backlight_percent_; }
    M5StickCPlusPower& m5StickCPlusPower() { return m5_power_; }

private:
    BoardConfig board_;
    M5StickCPlusPower m5_power_;
    uint8_t backlight_percent_ = 90;
};

} // namespace arfi
