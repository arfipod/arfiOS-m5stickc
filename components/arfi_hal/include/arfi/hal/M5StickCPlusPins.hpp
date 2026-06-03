#pragma once

#include "driver/gpio.h"
#include "driver/i2c.h"
#include "driver/spi_master.h"

namespace arfi::m5stickcplus {

static constexpr spi_host_device_t kLcdSpiHost = SPI2_HOST;

static constexpr gpio_num_t kLcdMosi = GPIO_NUM_15;
static constexpr gpio_num_t kLcdSclk = GPIO_NUM_13;
static constexpr gpio_num_t kLcdDc = GPIO_NUM_23;
static constexpr gpio_num_t kLcdReset = GPIO_NUM_18;
static constexpr gpio_num_t kLcdCs = GPIO_NUM_5;

static constexpr int kLcdPixelClockHz = 40 * 1000 * 1000;
static constexpr int kLcdCommandBits = 8;
static constexpr int kLcdParameterBits = 8;
static constexpr int kLcdTransQueueDepth = 10;

static constexpr int kDisplayWidth = 240;
static constexpr int kDisplayHeight = 135;

// ST7789 135x240 panels usually use an internal 240x320 address space.
// These offsets are the initial M5StickC Plus defaults. Adjust if your panel is shifted.
static constexpr int kLcdGapX = 40;
static constexpr int kLcdGapY = 52;
static constexpr bool kLcdSwapXY = true;
static constexpr bool kLcdMirrorX = true;
static constexpr bool kLcdMirrorY = false;

static constexpr gpio_num_t kButtonA = GPIO_NUM_37;
static constexpr gpio_num_t kButtonB = GPIO_NUM_39;
static constexpr gpio_num_t kIrLed = GPIO_NUM_9;
static constexpr gpio_num_t kRedLed = GPIO_NUM_10;

static constexpr i2c_port_t kI2cPort = I2C_NUM_0;
static constexpr gpio_num_t kI2cSda = GPIO_NUM_21;
static constexpr gpio_num_t kI2cScl = GPIO_NUM_22;
static constexpr uint32_t kI2cFrequencyHz = 400000;
static constexpr uint8_t kAxp192Address = 0x34;
static constexpr uint8_t kMpu6886Address = 0x68;

} // namespace arfi::m5stickcplus
