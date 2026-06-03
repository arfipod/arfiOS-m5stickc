#include "arfi/hal/M5StickCPlusPower.hpp"
#include "arfi/hal/M5StickCPlusPins.hpp"

#include "driver/i2c.h"
#include "esp_check.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

namespace arfi {

static const char* TAG = "m5_power";

esp_err_t M5StickCPlusPower::begin(const BoardConfig&) {
    if (initialized_) {
        return ESP_OK;
    }

    ESP_RETURN_ON_ERROR(initI2c_(), TAG, "I2C init failed");

    // Minimal AXP192 setup for M5StickC Plus display rails.
    // 0x28 controls LDO2/LDO3 voltage. 0xFF is a common safe high setting for the TFT rails.
    ESP_RETURN_ON_ERROR(writeReg_(0x28, 0xFF), TAG, "AXP192 LDO voltage setup failed");

    // 0x12 is the output power control register. OR with 0x0C to enable display-related LDOs.
    ESP_RETURN_ON_ERROR(updateReg_(0x12, 0x00, 0x0C), TAG, "AXP192 output enable failed");

    // Enable ADC channels for future power diagnostics. This is harmless for v0.1.
    ESP_RETURN_ON_ERROR(writeReg_(0x82, 0xFF), TAG, "AXP192 ADC setup failed");

    initialized_ = true;
    ESP_LOGI(TAG, "AXP192 initialized");
    return ESP_OK;
}

esp_err_t M5StickCPlusPower::setBacklightPercent(uint8_t percent) {
    if (!initialized_) {
        return ESP_ERR_INVALID_STATE;
    }

    if (percent == 0) {
        return updateReg_(0x12, 0x04, 0x00);
    }

    if (percent > 100) {
        percent = 100;
    }

    // Approximate brightness by LDO2 voltage. The display is small; keep a conservative lower bound.
    const uint8_t level = static_cast<uint8_t>(6 + ((percent * 9) / 100)); // 6..15

    uint8_t reg28 = 0;
    ESP_RETURN_ON_ERROR(readReg_(0x28, reg28), TAG, "AXP192 read 0x28 failed");
    reg28 = static_cast<uint8_t>((reg28 & 0x0F) | (level << 4));
    ESP_RETURN_ON_ERROR(writeReg_(0x28, reg28), TAG, "AXP192 write 0x28 failed");

    return updateReg_(0x12, 0x00, 0x04);
}

esp_err_t M5StickCPlusPower::initI2c_() {
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

esp_err_t M5StickCPlusPower::writeReg_(uint8_t reg, uint8_t value) {
    uint8_t data[2] = {reg, value};
    return i2c_master_write_to_device(
        m5stickcplus::kI2cPort,
        m5stickcplus::kAxp192Address,
        data,
        sizeof(data),
        pdMS_TO_TICKS(100));
}

esp_err_t M5StickCPlusPower::readReg_(uint8_t reg, uint8_t& value) {
    return i2c_master_write_read_device(
        m5stickcplus::kI2cPort,
        m5stickcplus::kAxp192Address,
        &reg,
        1,
        &value,
        1,
        pdMS_TO_TICKS(100));
}

esp_err_t M5StickCPlusPower::updateReg_(uint8_t reg, uint8_t clear_mask, uint8_t set_mask) {
    uint8_t value = 0;
    ESP_RETURN_ON_ERROR(readReg_(reg, value), TAG, "AXP192 read failed");
    value = static_cast<uint8_t>((value & ~clear_mask) | set_mask);
    return writeReg_(reg, value);
}

} // namespace arfi
