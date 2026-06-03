# arfiOS App Model

Apps in arfiOS v0.1 are native C++ classes compiled into the firmware.

## Base interface

```cpp
class App {
public:
    virtual const char* id() const = 0;
    virtual const char* name() const = 0;

    virtual void onEnter() {}
    virtual void onExit() {}

    virtual void update(uint32_t dt_ms) = 0;
    virtual void render(Canvas& canvas) = 0;
    virtual bool handleInput(const InputEvent& event) = 0;
};
```

## Lifecycle

```text
boot
  -> System registers apps
  -> LauncherApp enters
  -> user selects app
  -> current app exits
  -> selected app enters
  -> selected app receives update/render/input
  -> user returns home
  -> selected app exits
  -> LauncherApp enters
```

## App descriptor

Apps are exposed to the launcher through descriptors:

```cpp
struct AppDescriptor {
    const char* id;
    const char* name;
    const char* category;
    const char* glyph;
    App* instance;
};
```

`glyph` is a short text icon used by the current simple UI. Future versions can replace it with bitmap icons.

## Rules for apps

Apps should:

- use `SystemContext` for services;
- draw through `Canvas`;
- react to normalized `InputEvent` values;
- store settings through `SettingsService`;
- avoid blocking for long periods;
- avoid direct `gpio_*`, `nvs_*`, `esp_lcd_*`, `spi_*`, or `i2c_*` calls.

Apps should not:

- initialize hardware directly;
- own global hardware state;
- start one FreeRTOS task per app in v0.1;
- assume M5StickC Plus-specific buttons;
- assume Cardputer keyboard availability.

## Adding a new native app

1. Add `MyApp.hpp` and `MyApp.cpp` under `components/arfi_apps/my_app/`.
2. Implement `arfi::App`.
3. Add the source file to `components/arfi_apps/CMakeLists.txt`.
4. Add the app member to `System`.
5. Register it in `System::registerApps()`.

Example registration:

```cpp
registry_.add({"my_app", "My App", "Tools", "MY", &my_app_});
```
