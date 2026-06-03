#include "arfi/hal/M5StickCPlusImu.hpp"
#include "arfi/hal/M5StickCPlusPins.hpp"

#include "driver/i2c.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

namespace arfi {

static const char* TAG = "m5_imu";

namespace {
constexpr uint8_t kRegSelfTestXGyro = 0x00;
constexpr uint8_t kRegSmplrtDiv = 0x19;
constexpr uint8_t kRegConfig = 0x1A;
constexpr uint8_t kRegGyroConfig = 0x1B;
constexpr uint8_t kRegAccelConfig = 0x1C;
constexpr uint8_t kRegAccelConfig2 = 0x1D;
constexpr uint8_t kRegAccelXoutH = 0x3B;
constexpr uint8_t kRegPwrMgmt1 = 0x6B;
constexpr uint8_t kRegPwrMgmt2 = 0x6C;
constexpr uint8_t kRegWhoAmI = 0x75;

constexpr float kAccelScaleG = 1.0f / 8192.0f; // +/-4g
constexpr float kGyroScaleDps = 1.0f / 16.4f; // +/-2000 dps
} // namespace

esp_err_t M5StickCPlusImu::begin(const BoardConfig& board) {
    if (initialized_) {
        return ESP_OK;
    }

    if (board.type != BoardType::M5StickCPlus) {
        return ESP_ERR_NOT_SUPPORTED;
    }

    ESP_RETURN_ON_ERROR(initI2c_(), TAG, "I2C init failed");

    uint8_t who_am_i = 0;
    ESP_RETURN_ON_ERROR(readReg_(kRegWhoAmI, who_am_i), TAG, "MPU6886 WHO_AM_I read failed");
    if (who_am_i != 0x19) {
        ESP_LOGW(TAG, "Unexpected MPU6886 WHO_AM_I 0x%02x, continuing", who_am_i);
    }

    ESP_RETURN_ON_ERROR(writeReg_(kRegPwrMgmt1, 0x80), TAG, "MPU6886 reset failed");
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_RETURN_ON_ERROR(writeReg_(kRegPwrMgmt1, 0x01), TAG, "MPU6886 clock setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(kRegPwrMgmt2, 0x00), TAG, "MPU6886 sensor enable failed");
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_RETURN_ON_ERROR(writeReg_(kRegSmplrtDiv, 0x05), TAG, "MPU6886 sample rate setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(kRegConfig, 0x01), TAG, "MPU6886 filter setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(kRegGyroConfig, 0x18), TAG, "MPU6886 gyro range setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(kRegAccelConfig, 0x08), TAG, "MPU6886 accel range setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(kRegAccelConfig2, 0x01), TAG, "MPU6886 accel filter setup failed");

    // Reading this bank once after setup mirrors common MPU6886 init sequences and lets the bus settle.
    uint8_t ignored = 0;
    (void)readReg_(kRegSelfTestXGyro, ignored);

    initialized_ = true;
    ESP_LOGI(TAG, "MPU6886 initialized");
    return ESP_OK;
}

esp_err_t M5StickCPlusImu::read(M5StickCPlusImuSample& sample) {
    if (!initialized_) {
        return ESP_ERR_INVALID_STATE;
    }

    uint8_t data[14] = {};
    ESP_RETURN_ON_ERROR(readRegs_(kRegAccelXoutH, data, sizeof(data)), TAG, "MPU6886 sample read failed");

    const int16_t ax = toInt16_(data[0], data[1]);
    const int16_t ay = toInt16_(data[2], data[3]);
    const int16_t az = toInt16_(data[4], data[5]);
    const int16_t temp = toInt16_(data[6], data[7]);
    const int16_t gx = toInt16_(data[8], data[9]);
    const int16_t gy = toInt16_(data[10], data[11]);
    const int16_t gz = toInt16_(data[12], data[13]);

    sample.accel_x_g = static_cast<float>(ax) * kAccelScaleG;
    sample.accel_y_g = static_cast<float>(ay) * kAccelScaleG;
    sample.accel_z_g = static_cast<float>(az) * kAccelScaleG;
    sample.gyro_x_dps = static_cast<float>(gx) * kGyroScaleDps;
    sample.gyro_y_dps = static_cast<float>(gy) * kGyroScaleDps;
    sample.gyro_z_dps = static_cast<float>(gz) * kGyroScaleDps;
    sample.temperature_c = (static_cast<float>(temp) / 326.8f) + 25.0f;
    return ESP_OK;
}

esp_err_t M5StickCPlusImu::initI2c_() {
    i2c_config_t conf = {};
    conf.mode = I2C_MODE_MASTER;
    conf.sda_io_num = m5stickcplus::kI2cSda;
    conf.scl_io_num = m5stickcplus::kI2cScl;
    conf.sda_pullup_en = GPIO_PULLUP_ENABLE;
    conf.scl_pullup_en = GPIO_PULLUP_ENABLE;
    conf.master.clk_speed = m5stickcplus::kI2cFrequencyHz;
    conf.clk_flags = 0;

    esp_err_t err = i2c_param_config(m5stickcplus::kI2cPort, &conf);
    if (err != ESP_OK) {
        return err;
    }

    err = i2c_driver_install(m5stickcplus::kI2cPort, conf.mode, 0, 0, 0);
    if (err == ESP_ERR_INVALID_STATE) {
        return ESP_OK;
    }

    return err;
}

esp_err_t M5StickCPlusImu::writeReg_(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    return i2c_master_write_to_device(
        m5stickcplus::kI2cPort,
        m5stickcplus::kMpu6886Address,
        data,
        sizeof(data),
        pdMS_TO_TICKS(100));
}

esp_err_t M5StickCPlusImu::readReg_(uint8_t reg, uint8_t& value) {
    return i2c_master_write_read_device(
        m5stickcplus::kI2cPort,
        m5stickcplus::kMpu6886Address,
        &reg,
        1,
        &value,
        1,
        pdMS_TO_TICKS(100));
}

esp_err_t M5StickCPlusImu::readRegs_(uint8_t reg, uint8_t* data, size_t length) {
    return i2c_master_write_read_device(
        m5stickcplus::kI2cPort,
        m5stickcplus::kMpu6886Address,
        &reg,
        1,
        data,
        length,
        pdMS_TO_TICKS(100));
}

int16_t M5StickCPlusImu::toInt16_(uint8_t high, uint8_t low) {
    return static_cast<int16_t>((static_cast<uint16_t>(high) << 8) | low);
}

} // namespace arfi
