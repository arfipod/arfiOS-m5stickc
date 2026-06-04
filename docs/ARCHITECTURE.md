# arfiOS Architecture

arfiOS is a modular ESP-IDF firmware for M5Stack-style ESP32 devices, with a launcher experience and native foreground apps. It is not a general-purpose operating system: it does not provide user processes, virtual memory, dynamically loaded executables, or a stable plugin ABI. The `OS` name describes the product and the runtime metaphor: apps, shared services, a launcher, and a common UI layer.

The deployment unit is a single firmware image. Apps are compiled into that image and registered during boot.

## Goals

- Keep hardware access behind HAL adapters and services.
- Give apps a small, stable API through `SystemContext`.
- Render all UI through a shared 16-bit RGB565 `Canvas`.
- Provide a usable two-button launcher on M5StickC Plus.
- Leave a clean path for future targets such as Cardputer-Adv.
- Avoid one FreeRTOS task per app in v0.1.

## Layers

```mermaid
flowchart TD
    Main[main/main.cpp] --> Runtime[arfi_runtime<br/>System]
    Runtime --> Core[arfi_core<br/>App, AppManager, AppRegistry,<br/>SystemContext, InputEvent, AppIcon]
    Runtime --> Apps[arfi_apps<br/>Launcher, Settings, Diagnostics,<br/>Pomodoro, Nivel IMU, Barrido IR,<br/>REST API, Flappy Bird, About]
    Runtime --> Services[arfi_services<br/>Display, Input, Settings,<br/>Power, IMU, IR, Wi-Fi, REST]
    Apps --> Core
    Apps --> Services
    Apps --> UI[arfi_ui<br/>Canvas, Color, Theme,<br/>CoverFlowRenderer, ListRenderer]
    Services --> UI
    Services --> HAL[arfi_hal<br/>BoardConfig,<br/>M5StickCPlus adapters]
    HAL --> IDF[ESP-IDF<br/>FreeRTOS, GPIO, I2C, SPI,<br/>NVS, esp_lcd, LEDC, Wi-Fi,<br/>HTTP client, heap, partitions]
```

## Component Responsibilities

| Component | Responsibility | Must not do |
|---|---|---|
| `arfi_core` | Central contracts: app interface, registry, manager, normalized input, icons, context | Depend on M5StickC Plus details |
| `arfi_runtime` | Orchestrate boot, services, registered apps, and the main loop | Contain app-specific behavior |
| `arfi_hal` | Low-level board adapters and pin definitions | Expose raw APIs to apps |
| `arfi_services` | Safe shared APIs for hardware and system features | Render full app screens |
| `arfi_ui` | Canvas, colors, theme, launcher renderers | Access GPIO, NVS, or Wi-Fi |
| `arfi_apps` | Foreground user experiences | Initialize hardware directly |
| `main` | Entry point and delegation to `System` | Duplicate initialization logic |

## Main Class Diagram

```mermaid
classDiagram
    direction LR

    class App {
      <<abstract>>
      +id()
      +name()
      +onEnter()
      +onExit()
      +update(dt_ms)
      +render(canvas)
      +handleInput(event)
    }

    class System {
      +begin()
      +tick()
      -registerApps()
      -processInput()
      -shouldRender(now_ms)
    }

    class SystemContext {
      +board
      +app_manager
      +app_registry
      +display
      +input
      +settings
      +power
      +imu
      +ir
      +wifi
      +rest
    }

    class AppManager {
      +launch(next)
      +update(dt_ms)
      +render(canvas)
      +handleInput(event)
      +current()
    }

    class AppRegistry {
      +add(descriptor)
      +at(index)
      +findAppById(id)
      +count()
    }

    class AppDescriptor {
      +id
      +name
      +category
      +glyph
      +instance
      +icon
    }

    class AppIcon {
      +width
      +height
      +format
      +data
      +foreground_rgb565
      +background_rgb565
      +has_transparency
      +transparent_rgb565
      +valid()
    }

    class Canvas {
      +clear(color)
      +pixel(x,y,color)
      +fillRect(x,y,w,h,color)
      +drawRect(x,y,w,h,color)
      +drawText(x,y,text,color,scale)
      +drawCenteredText(cx,y,text,color,scale)
      +drawAppIcon(icon,x,y,scale)
    }

    class Theme {
      +background
      +surface
      +surface_alt
      +text
      +muted
      +accent
      +warning
    }

    System o-- SystemContext
    System o-- AppManager
    System o-- AppRegistry
    System o-- DisplayService
    System o-- InputService
    System o-- SettingsService
    System o-- PowerService
    System o-- ImuService
    System o-- IrService
    System o-- WifiService
    System o-- RestService
    System o-- LauncherApp
    System o-- SettingsApp
    System o-- DiagnosticsApp
    System o-- PomodoroApp
    System o-- ImuLevelApp
    System o-- IrSweepApp
    System o-- RestReaderApp
    System o-- FlappyBirdApp
    System o-- AboutApp

    AppManager --> App
    AppRegistry o-- AppDescriptor
    AppDescriptor --> App
    AppDescriptor --> AppIcon
    App --> SystemContext
    App --> Canvas
    Canvas --> AppIcon

    App <|-- LauncherApp
    App <|-- SettingsApp
    App <|-- DiagnosticsApp
    App <|-- PomodoroApp
    App <|-- ImuLevelApp
    App <|-- IrSweepApp
    App <|-- RestReaderApp
    App <|-- FlappyBirdApp
    App <|-- AboutApp
```

## Services And HAL Class Diagram

```mermaid
classDiagram
    direction LR

    class BoardConfig {
      +type
      +name
      +display_width
      +display_height
      +has_keyboard
      +has_micro_sd
      +has_audio_codec
      +has_imu
      +has_ir
    }

    class DisplayService {
      +begin(board,power)
      +canvas()
      +flush()
      +width()
      +height()
    }

    class InputService {
      +begin(board)
      +update(now_ms)
      +poll(event)
    }

    class SettingsService {
      +begin()
      +getInt(key,fallback)
      +setInt(key,value)
      +getBool(key,fallback)
      +setBool(key,value)
      +getString(key,fallback)
      +setString(key,value)
    }

    class PowerService {
      +begin(board)
      +setBacklightPercent(percent)
      +backlightPercent()
    }

    class ImuService {
      +begin(board)
      +read(sample)
      +available()
      +lastError()
    }

    class IrService {
      +begin(board)
      +update(now_ms)
      +emitBurst(carrier_hz,duration_ms)
      +stop()
      +available()
      +active()
    }

    class WifiService {
      +begin()
      +connect(timeout_ms)
      +initialized()
      +connected()
      +hasCredentials()
      +ssid()
      +lastError()
    }

    class RestService {
      +begin(wifi)
      +get(url,response,max_body_bytes)
      +available()
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
      +initialized()
    }

    class M5StickCPlusIr {
      +begin(board)
      +emitCarrier(carrier_hz)
      +stop()
      +active()
      +carrierHz()
    }

    DisplayService --> BoardConfig
    InputService --> BoardConfig
    PowerService --> BoardConfig
    ImuService --> BoardConfig
    IrService --> BoardConfig
    DisplayService o-- M5StickCPlusDisplay
    PowerService o-- M5StickCPlusPower
    ImuService o-- M5StickCPlusImu
    IrService o-- M5StickCPlusIr
    RestService --> WifiService
```

## Boot Sequence

```mermaid
sequenceDiagram
    autonumber
    participant Main as main
    participant System as System
    participant NVS as NVS
    participant Settings as SettingsService
    participant Power as PowerService
    participant Display as DisplayService
    participant Input as InputService
    participant IMU as ImuService
    participant IR as IrService
    participant WiFi as WifiService
    participant REST as RestService
    participant Registry as AppRegistry
    participant Manager as AppManager
    participant Launcher as LauncherApp

    Main->>System: begin()
    System->>NVS: nvs_flash_init()
    alt NVS pages are invalid or from a newer version
        System->>NVS: nvs_flash_erase()
        System->>NVS: nvs_flash_init()
    end
    System->>Settings: begin()
    System->>Power: begin(board)
    System->>Display: begin(board, power)
    System->>Settings: getInt("ui_bright", 90)
    System->>Power: setBacklightPercent(value)
    System->>Input: begin(board)
    System->>IMU: begin(board)
    Note over System,IMU: If it fails, boot continues with a warning
    System->>IR: begin(board)
    Note over System,IR: Optional service
    System->>WiFi: begin()
    Note over System,WiFi: Wi-Fi may be initialized without credentials
    System->>REST: begin(wifi)
    System->>Registry: add(descriptor + icon)
    System->>Manager: launch(launcher)
    Manager->>Launcher: onEnter()
    Launcher->>Settings: getInt("lnch_sel", 0)
    Launcher->>Settings: getInt("lnch_view", 0)
    System-->>Main: ESP_OK
```

## Main Loop Sequence

`System::tick()` is called continuously by `main`. In v0.1, the app runtime runs on the foreground loop; arfiOS does not create one task per app.

```mermaid
sequenceDiagram
    autonumber
    participant Main as main loop
    participant System as System
    participant Timer as esp_timer
    participant Input as InputService
    participant IR as IrService
    participant Manager as AppManager
    participant App as Current App
    participant Display as DisplayService
    participant Canvas as Canvas

    loop forever
        Main->>System: tick()
        System->>Timer: nowMs()
        Timer-->>System: now_ms
        System->>Input: update(now_ms)
        System->>IR: update(now_ms)
        System->>Input: poll(event)
        loop while events are available
            Input-->>System: InputEvent
            alt Current app is not launcher and event is Primary LongPress
                System->>Manager: launch(LauncherApp)
            else Normal event
                System->>Manager: handleInput(event)
                Manager->>App: handleInput(event)
            end
            System->>Input: poll(event)
        end
        System->>Manager: update(dt_ms)
        Manager->>App: update(dt_ms)
        alt now_ms - last_render_ms >= 33
            System->>Display: canvas()
            Display-->>System: Canvas
            System->>Manager: render(canvas)
            Manager->>App: render(canvas)
            App->>Canvas: draw...
            System->>Display: flush()
        end
    end
```

## App Switch Sequence

```mermaid
sequenceDiagram
    autonumber
    participant User as User
    participant Input as InputService
    participant System as System
    participant Launcher as LauncherApp
    participant Registry as AppRegistry
    participant Manager as AppManager
    participant Current as Current app
    participant Next as Selected app

    User->>Input: Button A short in launcher
    Input-->>System: Primary + ShortPress
    System->>Manager: handleInput(event)
    Manager->>Launcher: handleInput(event)
    Launcher->>Registry: at(selected_index)
    Registry-->>Launcher: AppDescriptor
    Launcher->>Manager: launch(descriptor.instance)
    Manager->>Current: onExit()
    Manager->>Next: onEnter()
    Note over System,Next: From the next tick onward, update/render/input target Next
```

## Global Return To Launcher

```mermaid
sequenceDiagram
    autonumber
    participant User as User
    participant Input as InputService
    participant System as System
    participant Manager as AppManager
    participant App as Foreground app
    participant Launcher as LauncherApp

    User->>Input: Button A long
    Input-->>System: Primary + LongPress
    System->>Manager: current()
    alt current != LauncherApp
        System->>Manager: launch(LauncherApp)
        Manager->>App: onExit()
        Manager->>Launcher: onEnter()
    else current == LauncherApp
        System->>Manager: handleInput(event)
        Manager->>Launcher: handleInput(event)
        Launcher->>Launcher: toggleViewMode()
    end
```

## Launcher Render And Icon Sequence

```mermaid
sequenceDiagram
    autonumber
    participant Launcher as LauncherApp
    participant Renderer as CoverFlow/List Renderer
    participant Registry as AppRegistry
    participant Canvas as Canvas
    participant Icon as AppIcon

    Launcher->>Renderer: render(canvas, registry, selected, theme)
    Renderer->>Registry: at(index)
    Registry-->>Renderer: AppDescriptor
    alt descriptor.icon is valid
        Renderer->>Canvas: drawAppIcon(icon, x, y, scale)
        Canvas->>Icon: Read Mono1 or RGB565 from data
        loop per pixel
            Canvas->>Canvas: fillRect(scaled pixel)
        end
    else no icon or icon does not fit
        Renderer->>Canvas: drawCenteredText(glyph)
    end
```

## REST API Sequence

The `REST API` app performs a synchronous HTTP GET and caps the received body. If Wi-Fi is not connected, `RestService` asks `WifiService` to connect before running the HTTP client.

```mermaid
sequenceDiagram
    autonumber
    participant User as User
    participant RestApp as RestReaderApp
    participant Settings as SettingsService
    participant Rest as RestService
    participant WiFi as WifiService
    participant EventLoop as ESP event loop
    participant HTTP as esp_http_client
    participant JSON as cJSON

    RestApp->>Settings: getString("rest_url", ARFI_REST_URL)
    User->>RestApp: Primary + ShortPress
    RestApp->>Rest: get(endpoint, response)
    Rest->>WiFi: connect()
    alt Already connected
        WiFi-->>Rest: ESP_OK
    else Needs connection
        WiFi->>WiFi: esp_wifi_set_config()
        WiFi->>WiFi: esp_wifi_start()
        WiFi->>WiFi: esp_wifi_connect()
        EventLoop-->>WiFi: WIFI_EVENT_STA_DISCONNECTED or IP_EVENT_STA_GOT_IP
        WiFi-->>Rest: ESP_OK, ESP_FAIL, or ESP_ERR_TIMEOUT
    end
    alt Wi-Fi OK
        Rest->>HTTP: esp_http_client_init()
        Rest->>HTTP: esp_http_client_perform()
        loop HTTP_EVENT_ON_DATA
            HTTP-->>Rest: chunk
            Rest->>Rest: append until max_body_bytes
        end
        Rest->>HTTP: status_code, content_length
        Rest->>HTTP: cleanup()
        Rest-->>RestApp: RestResponse
        RestApp->>JSON: parse body
        RestApp->>RestApp: search reading/value/temperature/datetime/unixtime/message
    else Connection error
        Rest-->>RestApp: error
        RestApp->>RestApp: State::Error
    end
```

## IMU Sequence

```mermaid
sequenceDiagram
    autonumber
    participant System as System
    participant ImuService as ImuService
    participant M5IMU as M5StickCPlusImu
    participant App as ImuLevelApp
    participant Canvas as Canvas

    System->>ImuService: begin(board)
    ImuService->>M5IMU: begin(board)
    M5IMU-->>ImuService: ESP_OK or error
    loop every ~40 ms while the app is foreground
        System->>App: update(dt_ms)
        App->>ImuService: available()
        alt available
            App->>ImuService: read(sample)
            ImuService->>M5IMU: read(m5_sample)
            M5IMU-->>ImuService: accel, gyro, temp
            ImuService-->>App: ImuSample
            App->>App: compute roll/pitch and level offset
        else unavailable
            App->>App: store last_error
        end
        System->>App: render(canvas)
        App->>Canvas: Draw level view or raw view
    end
```

## IR Sweep Sequence

```mermaid
sequenceDiagram
    autonumber
    participant User as User
    participant App as IrSweepApp
    participant System as System
    participant IrService as IrService
    participant M5IR as M5StickCPlusIr

    User->>App: Primary + ShortPress
    App->>App: toggle running
    loop while running
        System->>IrService: update(now_ms)
        alt burst has expired
            IrService->>M5IR: stop()
        end
        System->>App: update(dt_ms)
        alt interval >= 120 ms
            App->>IrService: emitBurst(carrier_hz, 46 ms)
            IrService->>M5IR: emitCarrier(carrier_hz)
            IrService->>IrService: burst_end_ms = now + duration
            App->>App: advance carrier by current mode
        end
    end
    User->>App: Primary + ShortPress or exit
    App->>IrService: stop()
    IrService->>M5IR: stop()
```

## Settings And Persistence Sequence

```mermaid
sequenceDiagram
    autonumber
    participant User as User
    participant SettingsApp as SettingsApp
    participant Settings as SettingsService
    participant Power as PowerService
    participant Launcher as LauncherApp

    SettingsApp->>Settings: getInt("ui_bright", 90)
    SettingsApp->>Settings: getInt("lnch_view", 0)
    User->>SettingsApp: Secondary + ShortPress
    SettingsApp->>SettingsApp: change selected row
    User->>SettingsApp: Primary + ShortPress
    alt brightness row
        SettingsApp->>Settings: setInt("ui_bright", brightness)
        SettingsApp->>Power: setBacklightPercent(brightness)
    else launcher row
        SettingsApp->>Settings: setInt("lnch_view", mode)
    end
    Launcher->>Settings: getInt("lnch_view", 0)
```

## RAM And Flash Usage

`DiagnosticsApp` shows internal RAM usage/free values through `heap_caps_get_total_size(MALLOC_CAP_INTERNAL)` and `heap_caps_get_free_size(MALLOC_CAP_INTERNAL)`.

`AboutApp` shows RAM and an estimate of app partition flash usage. For flash it reads the ESP32 image header, sums image segments plus padding/checksum/digest, and compares that value against the app partition size.

```mermaid
sequenceDiagram
    autonumber
    participant About as AboutApp
    participant Heap as heap_caps
    participant Partitions as esp_partition
    participant Canvas as Canvas

    About->>Heap: heap_caps_get_total_size(INTERNAL)
    About->>Heap: heap_caps_get_free_size(INTERNAL)
    About->>Partitions: esp_partition_find_first(APP)
    About->>Partitions: esp_partition_read(header)
    loop per image segment
        About->>Partitions: esp_partition_read(segment_header)
        About->>About: add segment length
    end
    About->>Canvas: drawCenteredText("RAM ...")
    About->>Canvas: drawCenteredText("FLASH ...")
```

## Included Apps

| App | Category | Services used | Main state | Key controls |
|---|---|---|---|---|
| `LauncherApp` | System | `AppRegistry`, `AppManager`, `SettingsService` | selected app and view mode | B next, B long previous, A launch, A long toggle view |
| `SettingsApp` | System | `SettingsService`, `PowerService` | selected row, brightness, view mode | B row, A change |
| `DiagnosticsApp` | System | ESP-IDF heap | uptime, last input, RAM | buttons test events |
| `PomodoroApp` | Tools | UI | running, remaining seconds | A start/pause, B reset |
| `ImuLevelApp` | Tools | `ImuService` | sample, roll/pitch, zero, raw view | A zero, B raw |
| `IrSweepApp` | Tools | `IrService` | mode, carrier, running | A run/stop, B mode |
| `RestReaderApp` | Tools | `RestService`, `WifiService`, `SettingsService` | idle/fetching/done/error, raw view | A GET, B raw |
| `FlappyBirdApp` | Games | UI | ready/running/game over, pipes, score | A flap/start/retry |
| `AboutApp` | System | heap/partitions, board config | version, RAM, flash | informational |

## Memory Model

- The main framebuffer lives in DMA-capable internal RAM and uses `240 * 135 * 2 = 64,800` bytes.
- App icons are declared as `inline constexpr` with `ARFI_PROGMEM`; in ESP-IDF they live in flash/rodata.
- Apps are members of `System`; they are not dynamically allocated.
- `AppRegistry` stores descriptors by value, up to `kMaxApps = 32`.
- `InputService` uses a fixed 16-event ring buffer.
- `RestService` caps HTTP response bodies to 512 bytes by default.

## Error Handling

Boot distinguishes required services from optional services:

- Required: NVS, settings, power, display, input.
- Optional: IMU, IR, Wi-Fi, REST.

If an optional service fails, the firmware continues and the corresponding app shows `NOT READY` or `ERROR`.

## Architectural Rules

1. Apps do not initialize hardware; they use `SystemContext` and services.
2. Apps should return quickly from `update`, `render`, and `handleInput`.
3. The launcher is a normal app, but `System` gives it special handling for global return on `A long`.
4. Rendering always goes through `Canvas`; apps do not write directly to the display panel.
5. Board pins and panel offsets live in `arfi_hal`.
6. Persistent configuration goes through `SettingsService`.
7. Credentials and REST endpoints are passed through build flags or settings; real values should not be documented.
8. New hardware capabilities should be added in this order: HAL, service, app.

## Path For New Targets

To port arfiOS to another board:

1. Create or extend `BoardConfig`.
2. Add the pin map and concrete HAL adapters.
3. Implement or adapt `DisplayService`, `InputService`, and `PowerService`.
4. Preserve the `Canvas` contract so existing apps stay portable.
5. Add optional services according to capabilities (`has_imu`, `has_ir`, `has_keyboard`, `has_micro_sd`, etc.).
6. Revisit launcher renderers if the canvas is no longer `240x135`.

```mermaid
sequenceDiagram
    autonumber
    participant Dev as Developer
    participant Board as BoardConfig
    participant HAL as New HAL
    participant Services as Services
    participant Apps as Existing apps

    Dev->>Board: define type, name, and capabilities
    Dev->>HAL: implement display/input/power/peripherals
    Dev->>Services: connect services to the new BoardType
    Services-->>Apps: keep generic APIs stable
    Apps-->>Dev: reused without GPIO/SPI/I2C changes
```
