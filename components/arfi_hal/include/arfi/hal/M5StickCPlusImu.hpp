#pragma once

#include "arfi/hal/BoardConfig.hpp"

#include "esp_err.h"

#include <cstddef>
#include <cstdint>

namespace arfi {

struct M5StickCPlusImuSample {
    float accel_x_g = 0.0f;
    float accel_y_g = 0.0f;
    float accel_z_g = 0.0f;
    float gyro_x_dps = 0.0f;
    float gyro_y_dps = 0.0f;
    float gyro_z_dps = 0.0f;
    float temperature_c = 0.0f;
};

class M5StickCPlusImu {
public:
    esp_err_t begin(const BoardConfig& board);
    esp_err_t read(M5StickCPlusImuSample& sample);
    bool initialized() const { return initialized_; }

private:
    esp_err_t initI2c_();
    esp_err_t writeReg_(uint8_t reg, uint8_t value);
    esp_err_t readReg_(uint8_t reg, uint8_t& value);
    esp_err_t readRegs_(uint8_t reg, uint8_t* data, size_t length);
    static int16_t toInt16_(uint8_t high, uint8_t low);

    bool initialized_ = false;
};

} // namespace arfi
