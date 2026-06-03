#pragma once

#include "arfi/hal/BoardConfig.hpp"
#include "arfi/hal/M5StickCPlusImu.hpp"

#include "esp_err.h"

namespace arfi {

struct ImuSample {
    float accel_x_g = 0.0f;
    float accel_y_g = 0.0f;
    float accel_z_g = 0.0f;
    float gyro_x_dps = 0.0f;
    float gyro_y_dps = 0.0f;
    float gyro_z_dps = 0.0f;
    float temperature_c = 0.0f;
};

class ImuService {
public:
    esp_err_t begin(const BoardConfig& board);
    esp_err_t read(ImuSample& sample);

    bool available() const { return available_; }
    esp_err_t lastError() const { return last_error_; }

private:
    static ImuSample fromM5Sample_(const M5StickCPlusImuSample& sample);

    BoardConfig board_;
    M5StickCPlusImu m5_imu_;
    bool available_ = false;
    esp_err_t last_error_ = ESP_ERR_INVALID_STATE;
};

} // namespace arfi
