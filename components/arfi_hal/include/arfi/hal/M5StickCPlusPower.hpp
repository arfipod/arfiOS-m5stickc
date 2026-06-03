#pragma once

#include "esp_err.h"
#include "arfi/hal/BoardConfig.hpp"

#include <cstdint>

namespace arfi {

class M5StickCPlusPower {
public:
    esp_err_t begin(const BoardConfig& board);
    esp_err_t setBacklightPercent(uint8_t percent);

private:
    esp_err_t initI2c_();
    esp_err_t writeReg_(uint8_t reg, uint8_t value);
    esp_err_t readReg_(uint8_t reg, uint8_t& value);
    esp_err_t updateReg_(uint8_t reg, uint8_t clear_mask, uint8_t set_mask);

    bool initialized_ = false;
};

} // namespace arfi
