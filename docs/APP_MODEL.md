# arfiOS App Model

arfiOS v0.1 apps are native C++ classes compiled into the firmware. They are not external binaries and they are not runtime-loaded scripts. This keeps memory, safety, and lifecycle rules simple on ESP32.

Each app:

- inherits from `arfi::App`;
- receives a `SystemContext&` in its constructor;
- implements `update`, `render`, and `handleInput`;
- is exposed to the launcher through an `AppDescriptor`;
- may provide a flash/PROGMEM bitmap icon, with a text `glyph` fallback.

## Base Interface

```cpp
class App {
public:
    explicit App(SystemContext& ctx) : ctx_(ctx) {}
    virtual ~App() = default;

    virtual const char* id() const = 0;
    virtual const char* name() const = 0;

    virtual void onEnter() {}
    virtual void onExit() {}

    virtual void update(uint32_t dt_ms) = 0;
    virtual void render(Canvas& canvas) = 0;
    virtual bool handleInput(const InputEvent& event) = 0;

protected:
    SystemContext& ctx_;
};
```

## App Class Diagram

```mermaid
classDiagram
    direction TB

    class App {
      <<abstract>>
      #SystemContext ctx_
      +id()
      +name()
      +onEnter()
      +onExit()
      +update(dt_ms)
      +render(canvas)
      +handleInput(event)
    }

    class LauncherApp {
      -selected_index_
      -view_mode_
      -cover_flow_
      -list_
      -next()
      -previous()
      -launchSelected()
      -toggleViewMode()
    }

    class SettingsApp {
      -selected_row_
      -brightness_
      -view_mode_
      -changeSelected()
    }

    class DiagnosticsApp {
      -uptime_ms_
      -last_event_
    }

    class PomodoroApp {
      -running_
      -remaining_seconds_
      -accumulator_ms_
      -reset()
      -toggle()
    }

    class ImuLevelApp {
      -sample_
      -raw_view_
      -zero_x_g_
      -zero_y_g_
      -roll_deg_
      -pitch_deg_
      -zero()
      -updateAngles()
    }

    class IrSweepApp {
      -mode_
      -running_
      -carrier_hz_
      -direction_
      -bursts_
      -toggle()
      -nextMode()
      -emitStep()
    }

    class RestReaderApp {
      -state_
      -response_
      -endpoint_
      -reading_
      -raw_view_
      -fetch()
      -extractReading()
    }

    class FlappyBirdApp {
      -state_
      -bird_y_
      -velocity_
      -pipes_
      -score_
      -best_score_
      -reset()
      -flap()
      -updatePipe()
      -hitsPipe()
    }

    class AboutApp {
      -theme_
    }

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

## Lifecycle

```mermaid
sequenceDiagram
    autonumber
    participant System as System
    participant Manager as AppManager
    participant OldApp as Previous app
    participant NewApp as New app
    participant Input as InputService
    participant Display as DisplayService
    participant Canvas as Canvas

    System->>Manager: launch(NewApp)
    alt current app exists
        Manager->>OldApp: onExit()
    end
    Manager->>NewApp: onEnter()
    loop every tick
        System->>Input: poll(event)
        alt event available
            System->>Manager: handleInput(event)
            Manager->>NewApp: handleInput(event)
        end
        System->>Manager: update(dt_ms)
        Manager->>NewApp: update(dt_ms)
        alt render is due
            System->>Display: canvas()
            Display-->>System: Canvas
            System->>Manager: render(canvas)
            Manager->>NewApp: render(canvas)
            System->>Display: flush()
        end
    end
```

## AppManager

`AppManager` keeps a single `current_` pointer.

```mermaid
classDiagram
    class AppManager {
      -App current_
      +launch(next)
      +update(dt_ms)
      +render(canvas)
      +handleInput(event)
      +current()
    }

    class App {
      <<abstract>>
      +onEnter()
      +onExit()
      +update(dt_ms)
      +render(canvas)
      +handleInput(event)
    }

    AppManager --> App : current
```

Rules:

- `launch(nullptr)` does nothing.
- `launch(current_)` does not restart the app.
- On app switches, the manager calls `onExit()` on the previous app and then `onEnter()` on the new app.
- `update`, `render`, and `handleInput` are delegated only to the current app.

## AppRegistry And Descriptors

The launcher does not know concrete app classes. It reads descriptors from `AppRegistry`.

```cpp
struct AppDescriptor {
    const char* id = "";
    const char* name = "";
    const char* category = "";
    const char* glyph = "";
    App* instance = nullptr;
    const AppIcon* icon = nullptr;
};
```

```mermaid
classDiagram
    class AppRegistry {
      -apps_[32]
      -count_
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
      +valid()
    }

    class App {
      <<abstract>>
    }

    AppRegistry o-- AppDescriptor
    AppDescriptor --> App : instance
    AppDescriptor --> AppIcon : optional icon
```

`AppRegistry::add()` rejects:

- entries past `kMaxApps`;
- descriptors without `instance`;
- descriptors without `id`.

## Current App Registration

Registration happens in `System::registerApps()` after services are initialized.

```mermaid
sequenceDiagram
    autonumber
    participant System as System
    participant Registry as AppRegistry
    participant Icons as app_icons

    System->>Registry: add(Settings + app_icons::Settings)
    System->>Registry: add(Diagnostics + app_icons::Diagnostics)
    System->>Registry: add(Pomodoro + app_icons::Pomodoro)
    System->>Registry: add(Nivel IMU + app_icons::ImuLevel)
    System->>Registry: add(Barrido IR + app_icons::IrSweep)
    System->>Registry: add(Flappy Bird + app_icons::FlappyBird)
    System->>Registry: add(REST API + app_icons::Rest)
    System->>Registry: add(About + app_icons::About)
```

Current table:

| ID | Name | Category | Icon | App |
|---|---|---|---|---|
| `settings` | Settings | System | Mono1 | `SettingsApp` |
| `diagnostics` | Diagnostics | System | Mono1 | `DiagnosticsApp` |
| `pomodoro` | Pomodoro | Tools | Mono1 | `PomodoroApp` |
| `imu_level` | Nivel IMU | Tools | Mono1 | `ImuLevelApp` |
| `ir_sweep` | Barrido IR | Tools | Mono1 | `IrSweepApp` |
| `flappy_bird` | Flappy Bird | Games | RGB565 | `FlappyBirdApp` |
| `rest_reader` | REST API | Tools | Mono1 | `RestReaderApp` |
| `about` | About | System | Mono1 | `AboutApp` |

## PROGMEM Icons

The `icon` field is optional. If it is `nullptr`, invalid, or too large for the available launcher slot, the renderer uses `glyph`.

```mermaid
classDiagram
    class AppIcon {
      +uint8_t width
      +uint8_t height
      +AppIconFormat format
      +const void* data
      +uint16_t foreground_rgb565
      +uint16_t background_rgb565
      +bool has_transparency
      +uint16_t transparent_rgb565
      +valid()
    }

    class AppIconFormat {
      <<enumeration>>
      Rgb565
      Mono1
    }

    class Canvas {
      +drawAppIcon(icon,x,y,scale)
    }

    AppIcon --> AppIconFormat
    Canvas --> AppIcon
```

Supported formats:

- `AppIconFormat::Mono1`: 1-bit packed bitmap, MSB-first. A `1` bit draws `foreground_rgb565`; a `0` bit draws `background_rgb565` when transparency is disabled.
- `AppIconFormat::Rgb565`: row-major `uint16_t` pixel array. A transparent color can be skipped.

Mono1 example:

```cpp
inline constexpr uint8_t kMyIconBits[] ARFI_PROGMEM = {
    0b00111100,
    0b01000010,
    0b10100101,
    0b10000001,
    0b10100101,
    0b10011001,
    0b01000010,
    0b00111100,
};

inline constexpr AppIcon MyIcon = {
    8,
    8,
    AppIconFormat::Mono1,
    kMyIconBits,
    0xFFFF,
    0x0000,
    true,
    0x0000,
};
```

RGB565 example:

```cpp
inline constexpr uint16_t kMyColorIcon[] ARFI_PROGMEM = {
    0xF81F, 0xFFFF, 0xFFFF, 0xF81F,
    0xFFFF, 0x07E0, 0x07E0, 0xFFFF,
    0xFFFF, 0x07E0, 0x07E0, 0xFFFF,
    0xF81F, 0xFFFF, 0xFFFF, 0xF81F,
};

inline constexpr AppIcon MyColorIcon = {
    4,
    4,
    AppIconFormat::Rgb565,
    kMyColorIcon,
    0xFFFF,
    0x0000,
    true,
    0xF81F,
};
```

Render sequence:

```mermaid
sequenceDiagram
    autonumber
    participant Renderer as Launcher Renderer
    participant Descriptor as AppDescriptor
    participant Canvas as Canvas
    participant Icon as AppIcon data

    Renderer->>Descriptor: icon
    alt valid icon and scale fits
        Renderer->>Canvas: drawAppIcon(icon, x, y, scale)
        alt Mono1
            Canvas->>Icon: read packed bytes
            Canvas->>Canvas: draw active bits
        else RGB565
            Canvas->>Icon: read uint16_t pixels
            Canvas->>Canvas: draw non-transparent pixels
        end
    else fallback
        Renderer->>Canvas: drawCenteredText(glyph)
    end
```

## App Controls

| App | `Primary Short` | `Secondary Short` | `Secondary Long` | `Primary Long` |
|---|---|---|---|---|
| Launcher | open selected app | next app | previous app | toggle Cover/List |
| Settings | change selected row value | next row | previous row | return to launcher when not in launcher |
| Diagnostics | record A short | record B short | record B long | return to launcher |
| Pomodoro | start/pause | reset | reset/alternate behavior | return to launcher |
| Nivel IMU | set zero | toggle raw/level view | clear zero | return to launcher |
| Barrido IR | run/stop | change mode | reset carrier | return to launcher |
| REST API | GET | raw/body view | no action | return to launcher |
| Flappy Bird | flap/start/retry | no action | no action | return to launcher |
| About | no action | no action | no action | return to launcher |

`Primary Long` is intercepted by `System` when the current app is not `LauncherApp`, so most apps do not implement their own exit handling.

## Adding A Native App

1. Create `components/arfi_apps/include/arfi/apps/MyApp.hpp`.
2. Create `components/arfi_apps/my_app/MyApp.cpp`.
3. Inherit from `App` and implement the required methods.
4. Use services through `ctx_`.
5. Add the `.cpp` file and include directory to `components/arfi_apps/CMakeLists.txt`.
6. Add a `MyApp my_app_;` member to `System`.
7. Construct it in the `System` initializer list.
8. Optionally create an `AppIcon` in `AppIcons.hpp`.
9. Register the descriptor in `System::registerApps()`.

```cpp
registry_.add({"my_app", "My App", "Tools", "MY", &my_app_, &app_icons::MyIcon});
```

```mermaid
sequenceDiagram
    autonumber
    participant Dev as Developer
    participant Apps as arfi_apps
    participant CMake as CMakeLists
    participant SystemH as System.hpp
    participant SystemCpp as System.cpp
    participant Registry as AppRegistry
    participant Launcher as LauncherApp

    Dev->>Apps: create MyApp.hpp and MyApp.cpp
    Dev->>CMake: add source and include dir
    Dev->>SystemH: add MyApp member
    Dev->>SystemCpp: initialize my_app_(ctx_)
    Dev->>SystemCpp: registry_.add(descriptor)
    SystemCpp->>Registry: add(MyApp)
    Launcher->>Registry: read descriptor
    Launcher->>Launcher: show icon/glyph and allow launch
```

## App Rules

Apps should:

- use `SystemContext` for services;
- draw only through `Canvas`;
- react to `InputEvent`;
- store preferences through `SettingsService`;
- return quickly from `update`, `render`, and `handleInput`;
- tolerate optional services being unavailable.

Apps should not:

- initialize GPIO, SPI, I2C, NVS, LCD, Wi-Fi, or HTTP directly;
- assume concrete M5StickC Plus pins;
- allocate large buffers without a clear reason;
- create their own FreeRTOS task in v0.1;
- block the main loop with long waits.

## Recommended Patterns

### Small Local State

Keep state as simple members:

```cpp
bool running_ = false;
uint32_t accumulator_ms_ = 0;
Theme theme_ = defaultTheme();
```

### Controlled Polling

For sensors or animations:

```cpp
accumulator_ms_ += dt_ms;
if (accumulator_ms_ < kPeriodMs) {
    return;
}
accumulator_ms_ = 0;
```

### Optional Service Guard

```cpp
if (ctx_.imu == nullptr || !ctx_.imu->available()) {
    // Render a "not ready" state.
    return;
}
```

### Persistence On Enter/Exit

Apps with preferences use `onEnter()` to read state and `onExit()` or direct action handlers to write state.

```mermaid
sequenceDiagram
    autonumber
    participant App as App
    participant Settings as SettingsService

    App->>Settings: getInt/getString in onEnter()
    App->>App: user changes state
    App->>Settings: setInt/setString when confirmed or in onExit()
```
