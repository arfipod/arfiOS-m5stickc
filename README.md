# arfiOS

**arfiOS** is a small ESP-IDF launcher/runtime for M5Stack-style ESP32 devices.

It is **not** a general-purpose operating system. It is a single firmware image that provides:

- a visual application launcher;
- a native embedded application model;
- shared hardware services;
- persistent settings;
- a board abstraction layer;
- a clean path for future devices such as the Cardputer-Adv.

The first target is the **M5StickC Plus**. The first launcher supports both:

- **Cover Flow view**;
- **List view**.

This repository is intended to replace the initial Arduino/PlatformIO prototype with a documented ESP-IDF architecture.

## Status

Version: `0.1.0`

Implemented in this first version:

- ESP-IDF project layout;
- M5StickC Plus board configuration;
- ST7789 display service through ESP-IDF `esp_lcd`;
- AXP192 minimal power/backlight setup;
- MPU6886 IMU service;
- IR LED carrier/burst service;
- Wi-Fi STA and REST GET services;
- two-button input service with short, long, and double presses;
- app model: `App`, `AppManager`, `AppRegistry`;
- foreground-only native apps;
- Cover Flow launcher;
- List launcher;
- NVS-backed settings;
- demo apps: Settings, Diagnostics, Pomodoro, Nivel IMU, Barrido IR, REST API, Flappy Bird, About;
- PlatformIO build/upload environment for M5StickC Plus;
- GitHub Actions build workflow;
- GitHub Actions release workflow for tagged releases.

Not implemented yet:

- BLE;
- iDotMatrix control;
- SD card app manifests;
- dynamic binary loading;
- OTA firmware app switching;
- Cardputer-Adv support.

## Hardware target

Initial target:

```text
M5StickC Plus
ESP32-PICO-D4
ST7789v2 135x240 TFT
Button A: GPIO37
Button B: GPIO39
IR LED:   GPIO9
LCD MOSI: GPIO15
LCD SCLK: GPIO13
LCD DC:   GPIO23
LCD RST:  GPIO18
LCD CS:   GPIO5
MPU6886 I2C SDA: GPIO21
MPU6886 I2C SCL: GPIO22
AXP192 I2C SDA: GPIO21
AXP192 I2C SCL: GPIO22
```

arfiOS renders in landscape as `240x135`.

## Controls

The M5StickC Plus has only two regular user buttons, so arfiOS uses press semantics.

### Launcher controls

| Action | Meaning |
|---|---|
| Button B short | Next app |
| Button B long | Previous app |
| Button A short | Launch selected app |
| Button A long | Toggle Cover Flow/List view |
| Button B double | Toggle Cover Flow/List view |

### App controls

| Action | Meaning |
|---|---|
| Button A long | Return to launcher |
| Button A short | App primary action |
| Button B short | App secondary/next action |
| Button B long | App previous/back/alternate action |
| Button A double | App menu or alternate primary action |

## Build with PlatformIO

Install PlatformIO Core, then run:

```bash
pio run -e m5stickcplus
```

The included `platformio.ini` uses ESP-IDF through `espressif32@6.9.0`, which provides ESP-IDF `5.3.1`.

PlatformIO currently exposes an `m5stick-c` board profile, not a separate M5StickC Plus profile. arfiOS uses that ESP32/4 MB baseline and defines the M5StickC Plus hardware details in its own HAL.

### REST API app configuration

The REST API app uses compile-time settings so credentials are not committed by default:

```ini
build_flags =
    -D ARFI_BOARD_M5STICKCPLUS=1
    '-D ARFI_WIFI_SSID="your_ssid"'
    '-D ARFI_WIFI_PASSWORD="your_password"'
    '-D ARFI_REST_URL="http://your-api.local/reading"'
```

## Flash with PlatformIO

```bash
pio run -e m5stickcplus -t upload
```

The default upload port is `/dev/ttyUSB0`.

## Monitor with PlatformIO

```bash
pio device monitor -e m5stickcplus
```

See [docs/PLATFORMIO.md](docs/PLATFORMIO.md) for PlatformIO setup notes, partition details, and expected boot logs.

## Build with ESP-IDF

Install ESP-IDF, then run:

```bash
idf.py set-target esp32
idf.py build
```

## Flash with ESP-IDF

```bash
idf.py -p /dev/ttyUSB0 flash monitor
```

On macOS, the port may look like:

```bash
idf.py -p /dev/cu.usbserial-XXXX flash monitor
```

On Windows, it may look like:

```powershell
idf.py -p COM5 flash monitor
```

## Repository layout

```text
arfiOS/
  main/
    main.cpp
  components/
    arfi_core/       # App model, app manager, registry, system context
    arfi_runtime/    # System orchestration and main foreground loop
    arfi_hal/        # Board config and M5StickC Plus hardware adapters
    arfi_services/   # Display, input, settings, power, IMU, IR services
    arfi_ui/         # Canvas, theme, launcher renderers
    arfi_apps/       # Native apps
  docs/
    ARCHITECTURE.md
    APP_MODEL.md
    INPUT_MODEL.md
    HARDWARE.md
    PLATFORMIO.md
    ROADMAP.md
    CI_CD.md
  .github/workflows/
    ci.yml
    release.yml
```

## Design rules

1. Apps are native C++ modules compiled into the firmware.
2. Apps do not directly touch raw display, GPIO, NVS, BLE, or Wi-Fi APIs.
3. Apps receive access to the system through `SystemContext` and services.
4. Only one app is foreground at a time.
5. Background services are allowed; one FreeRTOS task per app is not part of v0.1.
6. Dynamic binary app loading is intentionally out of scope.
7. Future script/config apps may be loaded from SD on devices that have SD, such as Cardputer-Adv.

## Why not dynamic app binaries?

ESP32 firmware executes from mapped flash, not from arbitrary files in a normal filesystem. A safe dynamic binary model would require ABI stability, relocation, symbol management, partition strategy, memory rules, and reboot or loader mechanisms. arfiOS v0.1 deliberately avoids this and uses a native module model.

Large apps such as Doom-like experiments can later be handled as alternate OTA firmware entries, not as dynamically loaded in-process apps.

## Cardputer-Adv path

The architecture already anticipates Cardputer-Adv support:

- `BoardConfig` describes capabilities;
- input is normalized into `InputEvent`;
- apps use `Canvas`, not raw LCD APIs;
- settings are abstracted;
- display resolution is not hardcoded inside app logic;
- Cardputer keyboard support can be added through a new input HAL.

The future Cardputer-Adv target should add:

- keyboard input service;
- SD-backed storage service;
- audio service;
- richer launcher navigation;
- BLE iDotMatrix app;
- optional script app manifests.

## CI/CD

GitHub Actions are included:

- `ci.yml`: builds on push and pull request;
- `release.yml`: builds on version tags and publishes firmware artifacts.

To create a release:

```bash
git tag v0.1.0
git push origin v0.1.0
```

## License

MIT.
