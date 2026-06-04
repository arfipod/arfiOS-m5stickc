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
constexpr uint8_t kRegIntEnable = 0x38;
constexpr uint8_t kRegIntPinCfg = 0x37;
constexpr uint8_t kRegFifoEn = 0x23;
constexpr uint8_t kRegUserCtrl = 0x6A;
constexpr uint8_t kRegPwrMgmt1 = 0x6B;
constexpr uint8_t kRegPwrMgmt2 = 0x6C;
constexpr uint8_t kRegWhoAmI = 0x75;

constexpr uint8_t kSh200qWhoAmI = 0x30;
constexpr uint8_t kSh200qAccelConfig = 0x0E;
constexpr uint8_t kSh200qGyroConfig = 0x0F;
constexpr uint8_t kSh200qGyroDlpf = 0x11;
constexpr uint8_t kSh200qFifoConfig = 0x12;
constexpr uint8_t kSh200qAccelRange = 0x16;
constexpr uint8_t kSh200qGyroRange = 0x2B;
constexpr uint8_t kSh200qOutputAccel = 0x00;
constexpr uint8_t kSh200qOutputGyro = 0x06;
constexpr uint8_t kSh200qOutputTemp = 0x0C;
constexpr uint8_t kSh200qRegSet1 = 0xBA;
constexpr uint8_t kSh200qRegSet2 = 0xCA;
constexpr uint8_t kSh200qAdcReset = 0xC2;

constexpr float kMpu6886AccelScaleG = 8.0f / 32768.0f; // +/-8g
constexpr float kMpu6886GyroScaleDps = 2000.0f / 32768.0f; // +/-2000 dps
constexpr float kSh200qAccelScaleG = 8.0f / 32768.0f; // +/-8g
constexpr float kSh200qGyroScaleDps = 2000.0f / 32768.0f; // +/-2000 dps
} // namespace

esp_err_t M5StickCPlusImu::begin(const BoardConfig& board) {
    if (initialized_) {
        return ESP_OK;
    }

    if (board.type != BoardType::M5StickCPlus) {
        return ESP_ERR_NOT_SUPPORTED;
    }

    ESP_RETURN_ON_ERROR(initI2c_(), TAG, "I2C init failed");

    esp_err_t err = initMpu6886_();
    if (err == ESP_OK) {
        initialized_ = true;
        chip_ = Chip::Mpu6886;
        ESP_LOGI(TAG, "MPU6886 initialized");
        return ESP_OK;
    }
    ESP_LOGW(TAG, "MPU6886 init failed: %s, trying SH200Q", esp_err_to_name(err));

    err = initSh200q_();
    if (err == ESP_OK) {
        initialized_ = true;
        chip_ = Chip::Sh200q;
        ESP_LOGI(TAG, "SH200Q initialized");
        return ESP_OK;
    }

    chip_ = Chip::None;
    ESP_LOGW(TAG, "No supported IMU found: %s", esp_err_to_name(err));
    return err;
}

esp_err_t M5StickCPlusImu::initMpu6886_() {
    uint8_t who_am_i = 0;
    esp_err_t err = ESP_FAIL;
    for (uint8_t attempt = 0; attempt < 3; ++attempt) {
        err = readReg_(m5stickcplus::kMpu6886Address, kRegWhoAmI, who_am_i);
        if (err == ESP_OK) {
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(10));
    }
    ESP_RETURN_ON_ERROR(err, TAG, "MPU6886 WHO_AM_I read failed");
    if (who_am_i != 0x19) {
        ESP_LOGW(TAG, "Unexpected MPU6886 WHO_AM_I 0x%02x", who_am_i);
        return ESP_ERR_NOT_FOUND;
    }

    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegPwrMgmt1, 0x00), TAG, "MPU6886 wake failed");
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegPwrMgmt1, 0x80), TAG, "MPU6886 reset failed");
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegPwrMgmt1, 0x01), TAG, "MPU6886 clock setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegPwrMgmt2, 0x00), TAG, "MPU6886 sensor enable failed");
    vTaskDelay(pdMS_TO_TICKS(10));

    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegSmplrtDiv, 0x05), TAG, "MPU6886 sample rate setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegConfig, 0x01), TAG, "MPU6886 filter setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegGyroConfig, 0x18), TAG, "MPU6886 gyro range setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegAccelConfig, 0x10), TAG, "MPU6886 accel range setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegAccelConfig2, 0x00), TAG, "MPU6886 accel filter setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegUserCtrl, 0x00), TAG, "MPU6886 user control setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegFifoEn, 0x00), TAG, "MPU6886 FIFO setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegIntPinCfg, 0x22), TAG, "MPU6886 interrupt pin setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kMpu6886Address, kRegIntEnable, 0x01), TAG, "MPU6886 interrupt setup failed");

    // Reading this bank once after setup mirrors common MPU6886 init sequences and lets the bus settle.
    uint8_t ignored = 0;
    (void)readReg_(m5stickcplus::kMpu6886Address, kRegSelfTestXGyro, ignored);

    return ESP_OK;
}

esp_err_t M5StickCPlusImu::initSh200q_() {
    uint8_t who_am_i = 0;
    ESP_RETURN_ON_ERROR(
        readReg_(m5stickcplus::kSh200qAddress, kSh200qWhoAmI, who_am_i),
        TAG,
        "SH200Q WHO_AM_I read failed");
    if (who_am_i != 0x18) {
        ESP_LOGW(TAG, "Unexpected SH200Q WHO_AM_I 0x%02x", who_am_i);
        return ESP_ERR_NOT_FOUND;
    }

    uint8_t value = 0;
    ESP_RETURN_ON_ERROR(readReg_(m5stickcplus::kSh200qAddress, kSh200qAdcReset, value), TAG, "SH200Q ADC reset read failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, kSh200qAdcReset, value | 0x04), TAG, "SH200Q ADC reset high failed");
    vTaskDelay(pdMS_TO_TICKS(1));
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, kSh200qAdcReset, value & 0xFB), TAG, "SH200Q ADC reset low failed");

    ESP_RETURN_ON_ERROR(readReg_(m5stickcplus::kSh200qAddress, 0xD8, value), TAG, "SH200Q 0xD8 read failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, 0xD8, value | 0x80), TAG, "SH200Q 0xD8 high failed");
    vTaskDelay(pdMS_TO_TICKS(1));
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, 0xD8, value & 0x7F), TAG, "SH200Q 0xD8 low failed");

    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, 0x78, 0x61), TAG, "SH200Q 0x78 setup failed");
    vTaskDelay(pdMS_TO_TICKS(1));
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, 0x78, 0x00), TAG, "SH200Q 0x78 clear failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, kSh200qAccelConfig, 0x91), TAG, "SH200Q accel ODR setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, kSh200qGyroConfig, 0x13), TAG, "SH200Q gyro ODR setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, kSh200qGyroDlpf, 0x03), TAG, "SH200Q gyro filter setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, kSh200qFifoConfig, 0x00), TAG, "SH200Q FIFO setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, kSh200qAccelRange, 0x01), TAG, "SH200Q accel range setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, kSh200qGyroRange, 0x00), TAG, "SH200Q gyro range setup failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, kSh200qRegSet1, 0xC0), TAG, "SH200Q reg set1 failed");

    ESP_RETURN_ON_ERROR(readReg_(m5stickcplus::kSh200qAddress, kSh200qRegSet2, value), TAG, "SH200Q reg set2 read failed");
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, kSh200qRegSet2, value | 0x10), TAG, "SH200Q reg set2 high failed");
    vTaskDelay(pdMS_TO_TICKS(1));
    ESP_RETURN_ON_ERROR(writeReg_(m5stickcplus::kSh200qAddress, kSh200qRegSet2, value & 0xEF), TAG, "SH200Q reg set2 low failed");
    vTaskDelay(pdMS_TO_TICKS(10));

    return ESP_OK;
}

esp_err_t M5StickCPlusImu::read(M5StickCPlusImuSample& sample) {
    if (!initialized_) {
        return ESP_ERR_INVALID_STATE;
    }

    if (chip_ == Chip::Mpu6886) {
        return readMpu6886_(sample);
    }
    if (chip_ == Chip::Sh200q) {
        return readSh200q_(sample);
    }

    return ESP_ERR_INVALID_STATE;
}

esp_err_t M5StickCPlusImu::readMpu6886_(M5StickCPlusImuSample& sample) {
    uint8_t data[14] = {};
    ESP_RETURN_ON_ERROR(readRegs_(m5stickcplus::kMpu6886Address, kRegAccelXoutH, data, sizeof(data)), TAG, "MPU6886 sample read failed");

    const int16_t ax = toInt16_(data[0], data[1]);
    const int16_t ay = toInt16_(data[2], data[3]);
    const int16_t az = toInt16_(data[4], data[5]);
    const int16_t temp = toInt16_(data[6], data[7]);
    const int16_t gx = toInt16_(data[8], data[9]);
    const int16_t gy = toInt16_(data[10], data[11]);
    const int16_t gz = toInt16_(data[12], data[13]);

    sample.accel_x_g = static_cast<float>(ax) * kMpu6886AccelScaleG;
    sample.accel_y_g = static_cast<float>(ay) * kMpu6886AccelScaleG;
    sample.accel_z_g = static_cast<float>(az) * kMpu6886AccelScaleG;
    sample.gyro_x_dps = static_cast<float>(gx) * kMpu6886GyroScaleDps;
    sample.gyro_y_dps = static_cast<float>(gy) * kMpu6886GyroScaleDps;
    sample.gyro_z_dps = static_cast<float>(gz) * kMpu6886GyroScaleDps;
    sample.temperature_c = (static_cast<float>(temp) / 326.8f) + 25.0f;
    return ESP_OK;
}

esp_err_t M5StickCPlusImu::readSh200q_(M5StickCPlusImuSample& sample) {
    uint8_t accel[6] = {};
    uint8_t gyro[6] = {};
    uint8_t temp_data[2] = {};

    ESP_RETURN_ON_ERROR(readRegs_(m5stickcplus::kSh200qAddress, kSh200qOutputAccel, accel, sizeof(accel)), TAG, "SH200Q accel read failed");
    ESP_RETURN_ON_ERROR(readRegs_(m5stickcplus::kSh200qAddress, kSh200qOutputGyro, gyro, sizeof(gyro)), TAG, "SH200Q gyro read failed");
    ESP_RETURN_ON_ERROR(readRegs_(m5stickcplus::kSh200qAddress, kSh200qOutputTemp, temp_data, sizeof(temp_data)), TAG, "SH200Q temp read failed");

    const int16_t ax = toInt16Le_(accel[0], accel[1]);
    const int16_t ay = toInt16Le_(accel[2], accel[3]);
    const int16_t az = toInt16Le_(accel[4], accel[5]);
    const int16_t gx = toInt16Le_(gyro[0], gyro[1]);
    const int16_t gy = toInt16Le_(gyro[2], gyro[3]);
    const int16_t gz = toInt16Le_(gyro[4], gyro[5]);
    const int16_t temp = toInt16Le_(temp_data[0], temp_data[1]);

    sample.accel_x_g = static_cast<float>(ax) * kSh200qAccelScaleG;
    sample.accel_y_g = static_cast<float>(ay) * kSh200qAccelScaleG;
    sample.accel_z_g = static_cast<float>(az) * kSh200qAccelScaleG;
    sample.gyro_x_dps = static_cast<float>(gx) * kSh200qGyroScaleDps;
    sample.gyro_y_dps = static_cast<float>(gy) * kSh200qGyroScaleDps;
    sample.gyro_z_dps = static_cast<float>(gz) * kSh200qGyroScaleDps;
    sample.temperature_c = (static_cast<float>(temp) / 333.87f) + 21.0f;
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

esp_err_t M5StickCPlusImu::writeReg_(uint8_t address, uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    return i2c_master_write_to_device(
        m5stickcplus::kI2cPort,
        address,
        data,
        sizeof(data),
        pdMS_TO_TICKS(100));
}

esp_err_t M5StickCPlusImu::readReg_(uint8_t address, uint8_t reg, uint8_t& value) {
    return i2c_master_write_read_device(
        m5stickcplus::kI2cPort,
        address,
        &reg,
        1,
        &value,
        1,
        pdMS_TO_TICKS(100));
}

esp_err_t M5StickCPlusImu::readRegs_(uint8_t address, uint8_t reg, uint8_t* data, size_t length) {
    return i2c_master_write_read_device(
        m5stickcplus::kI2cPort,
        address,
        &reg,
        1,
        data,
        length,
        pdMS_TO_TICKS(100));
}

int16_t M5StickCPlusImu::toInt16_(uint8_t high, uint8_t low) {
    return static_cast<int16_t>((static_cast<uint16_t>(high) << 8) | low);
}

int16_t M5StickCPlusImu::toInt16Le_(uint8_t low, uint8_t high) {
    return static_cast<int16_t>((static_cast<uint16_t>(high) << 8) | low);
}

} // namespace arfi
