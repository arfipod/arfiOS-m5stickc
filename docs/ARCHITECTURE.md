# arfiOS Architecture

arfiOS is a single ESP-IDF firmware with a modular runtime architecture.

It is not a real operating system. It does not provide user processes, virtual memory, executable files, or dynamic binary loading. The term `OS` is used as a product name and as a metaphor for a launcher/runtime.

## Layers

```text
Application layer
  LauncherApp, SettingsApp, DiagnosticsApp, PomodoroApp, AboutApp

UI layer
  Canvas, Theme, CoverFlowRenderer, ListRenderer

Service layer
  DisplayService, InputService, SettingsService, PowerService

HAL layer
  M5StickCPlusDisplay, M5StickCPlusPower, BoardConfig

ESP-IDF layer
  FreeRTOS, GPIO, SPI, I2C, NVS, esp_lcd
```

## Core objects

### `App`

Base class implemented by every native application.

### `AppRegistry`

Stores app descriptors visible to the launcher.

### `AppManager`

Owns the current foreground app transition logic.

### `SystemContext`

Shared context passed to apps. It exposes service pointers and board configuration.

### `System`

Initializes services, registers apps, launches the launcher, and runs the foreground loop.

## Foreground app model

Only one app is foreground at a time.

```text
InputService -> System -> AppManager -> current App
DisplayService -> Canvas -> current App render -> LCD flush
```

Apps are expected to return quickly from `update`, `render`, and `handleInput`.

## Background services

Services may maintain internal state in the main loop or future FreeRTOS tasks.

For v0.1, arfiOS avoids creating one task per app. This keeps memory usage and lifecycle rules simple.

## Future extension points

- BLE service;
- Wi-Fi service;
- SD storage service;
- Cardputer-Adv keyboard input;
- script apps loaded from SD manifests;
- OTA firmware entries for large standalone apps.
