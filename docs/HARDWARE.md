# Hardware Notes

arfiOS v0.1 targets the M5StickC Plus. Hardware-specific details are isolated in `arfi_hal` and surfaced through services, so apps do not depend on raw GPIO, I2C, SPI, LCD, Wi-Fi, or LEDC APIs.

## Target Summary

| Item | Value |
|---|---|
| Board | M5StickC Plus |
| MCU | ESP32-PICO-D4 |
| Display | ST7789v2 TFT |
| Logical canvas | `240x135`, landscape |
| Power management | AXP192 |
| IMU | MPU6886 |
| IR transmitter | onboard IR LED on GPIO9 |
| Build board profile | PlatformIO `m5stick-c` ESP32/4 MB baseline |

## Pin Map

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

## Board Configuration

`BoardConfig` describes board capabilities independently from concrete drivers.

```mermaid
classDiagram
    class BoardConfig {
      +BoardType type
      +const char* name
      +uint16_t display_width
      +uint16_t display_height
      +bool has_keyboard
      +bool has_micro_sd
      +bool has_audio_codec
      +bool has_imu
      +bool has_ir
    }

    class BoardType {
      <<enumeration>>
      M5StickCPlus
      CardputerAdv
      Unknown
    }

    BoardConfig --> BoardType
```

For M5StickC Plus:

```text
type = M5StickCPlus
name = "M5StickC Plus"
display_width = 240
display_height = 135
has_keyboard = false
has_micro_sd = false
has_audio_codec = false
has_imu = true
has_ir = true
```

## Hardware Abstraction

```mermaid
classDiagram
    direction LR

    class System {
      +begin()
      +tick()
    }

    class DisplayService {
      +begin(board,power)
      +canvas()
      +flush()
    }

    class PowerService {
      +begin(board)
      +setBacklightPercent(percent)
    }

    class InputService {
      +begin(board)
      +update(now_ms)
      +poll(event)
    }

    class ImuService {
      +begin(board)
      +read(sample)
      +available()
    }

    class IrService {
      +begin(board)
      +emitBurst(carrier_hz,duration_ms)
      +stop()
      +update(now_ms)
    }

    class M5StickCPlusDisplay {
      +begin(board,power)
      +flush(pixels,width,height)
    }

    class M5StickCPlusPower {
      +begin(board)
      +setBacklightPercent(percent)
    }

    class M5StickCPlusImu {
      +begin(board)
      +read(sample)
    }

    class M5StickCPlusIr {
      +begin(board)
      +emitCarrier(carrier_hz)
      +stop()
    }

    System --> DisplayService
    System --> PowerService
    System --> InputService
    System --> ImuService
    System --> IrService
    DisplayService o-- M5StickCPlusDisplay
    PowerService o-- M5StickCPlusPower
    ImuService o-- M5StickCPlusImu
    IrService o-- M5StickCPlusIr
```

## Hardware Boot Sequence

```mermaid
sequenceDiagram
    autonumber
    participant System as System
    participant Power as PowerService
    participant M5Power as M5StickCPlusPower
    participant Display as DisplayService
    participant M5Display as M5StickCPlusDisplay
    participant Input as InputService
    participant IMU as ImuService
    participant M5IMU as M5StickCPlusImu
    participant IR as IrService
    participant M5IR as M5StickCPlusIr

    System->>Power: begin(board)
    Power->>M5Power: begin(board)
    M5Power-->>Power: ESP_OK
    System->>Display: begin(board, power)
    Display->>Display: allocate DMA framebuffer
    Display->>M5Display: begin(board, m5_power)
    M5Display-->>Display: ESP_OK
    System->>Input: begin(board)
    Input-->>System: ESP_OK
    System->>IMU: begin(board)
    IMU->>M5IMU: begin(board)
    M5IMU-->>IMU: ESP_OK or error
    System->>IR: begin(board)
    IR->>M5IR: begin(board)
    M5IR-->>IR: ESP_OK or error
```

## Display

The display is driven through ESP-IDF `esp_lcd` with the ST7789 panel driver. arfiOS renders into a logical `240x135` framebuffer and then flushes it to the physical panel.

Current ST7789 offsets and orientation:

```text
x_gap = 40
y_gap = 52
swap_xy = true
mirror_x = true
mirror_y = false
```

Some ST7789 batches may require different offsets. If the image is shifted, adjust the constants in:

```text
components/arfi_hal/include/arfi/hal/M5StickCPlusPins.hpp
```

Display render/flush sequence:

```mermaid
sequenceDiagram
    autonumber
    participant App as Current app
    participant Canvas as Canvas
    participant Display as DisplayService
    participant HAL as M5StickCPlusDisplay
    participant LCD as ST7789 panel

    App->>Canvas: drawText/fillRect/drawAppIcon
    Canvas->>Canvas: write RGB565 pixels into framebuffer
    App-->>Display: render complete
    Display->>HAL: flush(buffer, 240, 135)
    HAL->>LCD: esp_lcd_panel_draw_bitmap()
```

## Framebuffer

The framebuffer is allocated in internal DMA-capable memory:

```text
240 * 135 * 2 bytes = 64,800 bytes
```

`Canvas` does clipping in software, so apps can draw near edges without manually clipping every primitive.

## AXP192 Power

The M5StickC Plus uses an AXP192 PMU. arfiOS v0.1 performs a minimal setup:

- initializes the shared I2C bus;
- sets the LDO2/LDO3 voltage register to a safe high value;
- enables display-related power rails;
- enables ADCs for future battery support;
- exposes backlight percentage through `PowerService`.

Battery reading is not implemented in v0.1.

Backlight sequence:

```mermaid
sequenceDiagram
    autonumber
    participant Settings as SettingsApp
    participant Power as PowerService
    participant PMU as M5StickCPlusPower
    participant AXP as AXP192

    Settings->>Power: setBacklightPercent(percent)
    Power->>PMU: setBacklightPercent(percent)
    PMU->>AXP: update power/backlight register
```

## Shared I2C Bus

AXP192 and MPU6886 share I2C on GPIO21/GPIO22.

```mermaid
flowchart LR
    ESP32[ESP32 I2C0] --> SDA[GPIO21 SDA]
    ESP32 --> SCL[GPIO22 SCL]
    SDA --> AXP[AXP192 0x34]
    SCL --> AXP
    SDA --> MPU[MPU6886 0x68]
    SCL --> MPU
```

## MPU6886 IMU

The M5StickC Plus exposes the MPU6886 on the shared I2C bus. arfiOS initializes it in the HAL and exposes accelerometer, gyroscope, and temperature samples through `ImuService`.

The generic service type is:

```cpp
struct ImuSample {
    float accel_x_g;
    float accel_y_g;
    float accel_z_g;
    float gyro_x_dps;
    float gyro_y_dps;
    float gyro_z_dps;
    float temperature_c;
};
```

Read sequence:

```mermaid
sequenceDiagram
    autonumber
    participant App as ImuLevelApp
    participant Service as ImuService
    participant HAL as M5StickCPlusImu
    participant MPU as MPU6886

    App->>Service: read(sample)
    Service->>HAL: read(m5_sample)
    HAL->>MPU: read accel/gyro/temp registers
    MPU-->>HAL: raw register bytes
    HAL->>HAL: convert raw values
    HAL-->>Service: M5StickCPlusImuSample
    Service-->>App: ImuSample
```

## IR Transmitter

The onboard IR transmitter is driven from GPIO9. arfiOS uses LEDC to generate short carrier bursts between 30 kHz and 60 kHz through `IrService`.

The service caps burst duration to 500 ms and stops the carrier automatically when `IrService::update(now_ms)` sees the burst end time has passed.

```mermaid
sequenceDiagram
    autonumber
    participant App as IrSweepApp
    participant Service as IrService
    participant HAL as M5StickCPlusIr
    participant LEDC as ESP-IDF LEDC
    participant Pin as GPIO9 IR LED

    App->>Service: emitBurst(carrier_hz, duration_ms)
    Service->>HAL: emitCarrier(carrier_hz)
    HAL->>LEDC: configure frequency/duty
    LEDC->>Pin: carrier output
    Service->>Service: burst_end_ms = now + duration
    Service->>HAL: stop()
    HAL->>LEDC: stop carrier
```

## Buttons

Button A and Button B are active-low and mapped by `InputService`:

| Button | GPIO | arfiOS key |
|---|---:|---|
| A | GPIO37 | `Primary` |
| B | GPIO39 | `Secondary` |

See [INPUT_MODEL.md](INPUT_MODEL.md) for debounce, short press, long press, and double press behavior.

## Hardware Extension Rules

When adding a new peripheral:

1. Add board capability fields to `BoardConfig` if needed.
2. Add pins and device constants under `arfi_hal`.
3. Implement a concrete HAL adapter for the board.
4. Add a service that exposes a board-independent API.
5. Inject the service through `SystemContext`.
6. Build apps against the service, not the HAL.

```mermaid
sequenceDiagram
    autonumber
    participant Dev as Developer
    participant HAL as arfi_hal
    participant Service as arfi_services
    participant Context as SystemContext
    participant App as App

    Dev->>HAL: add concrete hardware adapter
    Dev->>Service: wrap HAL in generic service
    Dev->>Context: expose service pointer
    App->>Context: access service
    Context-->>App: board-independent API
```
