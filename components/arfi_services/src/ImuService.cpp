#include "arfi/services/ImuService.hpp"

namespace arfi {

esp_err_t ImuService::begin(const BoardConfig& board) {
    board_ = board;
    available_ = false;

    if (!board.has_imu) {
        last_error_ = ESP_ERR_NOT_SUPPORTED;
        return last_error_;
    }

    if (board.type == BoardType::M5StickCPlus) {
        last_error_ = m5_imu_.begin(board);
        available_ = last_error_ == ESP_OK;
        return last_error_;
    }

    last_error_ = ESP_ERR_NOT_SUPPORTED;
    return last_error_;
}

esp_err_t ImuService::read(ImuSample& sample) {
    if (!available_) {
        return last_error_;
    }

    if (board_.type == BoardType::M5StickCPlus) {
        M5StickCPlusImuSample m5_sample;
        last_error_ = m5_imu_.read(m5_sample);
        if (last_error_ == ESP_OK) {
            sample = fromM5Sample_(m5_sample);
        }
        return last_error_;
    }

    last_error_ = ESP_ERR_NOT_SUPPORTED;
    return last_error_;
}

ImuSample ImuService::fromM5Sample_(const M5StickCPlusImuSample& sample) {
    ImuSample generic;
    generic.accel_x_g = sample.accel_x_g;
    generic.accel_y_g = sample.accel_y_g;
    generic.accel_z_g = sample.accel_z_g;
    generic.gyro_x_dps = sample.gyro_x_dps;
    generic.gyro_y_dps = sample.gyro_y_dps;
    generic.gyro_z_dps = sample.gyro_z_dps;
    generic.temperature_c = sample.temperature_c;
    return generic;
}

} // namespace arfi
