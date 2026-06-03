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

AXP192 SDA: GPIO21
AXP192 SCL: GPIO22
```

The display is driven through ESP-IDF `esp_lcd` with the ST7789 panel driver.

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

## Button electrical behavior

GPIO37 and GPIO39 are input-only GPIOs. arfiOS assumes active-low button behavior and does not enable internal pull-ups. The board provides the needed hardware behavior.
