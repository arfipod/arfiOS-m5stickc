# Hardware Notes

## M5StickC Plus pin map used by arfiOS v0.1

```text
LCD MOSI: GPIO15
LCD SCLK: GPIO13
LCD DC:   GPIO23
LCD RST:  GPIO18
LCD CS:   GPIO5

Button A: GPIO37
Button B: GPIO39
IR LED:   GPIO9

AXP192 SDA: GPIO21
AXP192 SCL: GPIO22
MPU6886 SDA: GPIO21
MPU6886 SCL: GPIO22
```

The display is driven through ESP-IDF `esp_lcd` with the ST7789 panel driver.
The IMU and AXP192 share the same I2C bus.

## Display orientation

arfiOS uses a logical canvas of `240x135` pixels.

The initial M5StickC Plus ST7789 offset defaults are:

```text
x_gap = 40
y_gap = 52
swap_xy = true
mirror_x = true
mirror_y = false
```

Some ST7789 batches require different offsets. If the image is shifted, adjust the constants in:

```text
components/arfi_hal/include/arfi/hal/M5StickCPlusPins.hpp
```

## AXP192

The M5StickC Plus uses an AXP192 PMU. arfiOS v0.1 performs a minimal setup:

- initialize I2C;
- set LDO2/LDO3 voltage register to a safe high value;
- enable display-related power rails;
- enable ADCs for future battery support.

Battery reading is not implemented in v0.1.

## MPU6886 IMU

The M5StickC Plus exposes the MPU6886 on the shared I2C bus. arfiOS initializes it in the HAL and exposes accelerometer, gyroscope, and temperature samples through `ImuService`.

## IR transmitter

The board IR transmitter is driven from GPIO9. arfiOS uses LEDC to generate short carrier bursts between 30 kHz and 60 kHz through `IrService`.

## Button electrical behavior

GPIO37 and GPIO39 are input-only GPIOs. arfiOS assumes active-low button behavior and does not enable internal pull-ups. The board provides the needed hardware behavior.
