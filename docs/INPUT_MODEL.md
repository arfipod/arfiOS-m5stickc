# arfiOS Input Model

arfiOS normalizes physical buttons and future keyboards into common input events. The first target, M5StickC Plus, has only two user buttons, so the firmware uses press semantics: short press, long press, and double press.

## Goals

- Keep apps independent from GPIO37/GPIO39.
- Give inputs semantic names such as `Primary`, `Secondary`, `Home`, `Back`, arrows, and characters.
- Debounce physical button noise.
- Detect `ShortPress`, `LongPress`, `DoublePress`, `Pressed`, and `Released`.
- Provide a global return-to-launcher gesture.

## Core Types

```cpp
enum class Key : uint8_t {
    Primary,
    Secondary,
    Home,
    Back,
    Up,
    Down,
    Left,
    Right,
    Character,
    Unknown,
};

enum class InputEventType : uint8_t {
    ShortPress,
    LongPress,
    DoublePress,
    Pressed,
    Released,
};

struct InputEvent {
    Key key;
    InputEventType type;
    char character;
    uint32_t timestamp_ms;
};
```

## Class Diagram

```mermaid
classDiagram
    direction LR

    class InputEvent {
      +Key key
      +InputEventType type
      +char character
      +uint32_t timestamp_ms
      +isShortPress()
      +isLongPress()
      +isDoublePress()
    }

    class Key {
      <<enumeration>>
      Primary
      Secondary
      Home
      Back
      Up
      Down
      Left
      Right
      Character
      Unknown
    }

    class InputEventType {
      <<enumeration>>
      ShortPress
      LongPress
      DoublePress
      Pressed
      Released
    }

    class InputService {
      -buttons_[2]
      -queue_[16]
      -queue_head_
      -queue_tail_
      +begin(board)
      +update(now_ms)
      +poll(event)
      -updateButton_(button, now_ms)
      -readPressed_(button)
      -push_(event)
    }

    class ButtonState {
      +gpio
      +key
      +active_low
      +stable_pressed
      +last_raw_pressed
      +last_raw_change_ms
      +press_start_ms
      +long_emitted
      +waiting_single
      +pending_single_ms
    }

    InputEvent --> Key
    InputEvent --> InputEventType
    InputService o-- ButtonState
    InputService o-- InputEvent
```

## M5StickC Plus Mapping

| Physical input | GPIO | Electrical behavior | arfiOS key |
|---|---:|---|---|
| Button A | GPIO37 | active-low | `Primary` |
| Button B | GPIO39 | active-low | `Secondary` |

GPIO37 and GPIO39 are input-only GPIOs. arfiOS does not enable internal pull-ups; it assumes the board provides the required electrical behavior.

## Detection Constants

| Constant | Value | Meaning |
|---|---:|---|
| `kDebounceMs` | 30 ms | minimum stable time before accepting a state change |
| `kLongPressMs` | 650 ms | time before emitting `LongPress` |
| `kDoublePressMs` | 300 ms | window that can convert two short presses into `DoublePress` |
| `kEventQueueSize` | 16 | ring-buffer event queue size |

## Initialization Sequence

```mermaid
sequenceDiagram
    autonumber
    participant System as System
    participant Input as InputService
    participant Board as BoardConfig
    participant GPIO as ESP-IDF GPIO

    System->>Input: begin(board)
    Input->>Board: type == M5StickCPlus
    Input->>Input: Button A => Primary, GPIO37
    Input->>Input: Button B => Secondary, GPIO39
    loop per button
        Input->>GPIO: gpio_config(input, no pulls, no interrupt)
        Input->>GPIO: gpio_get_level()
        GPIO-->>Input: initial state
        Input->>Input: stable_pressed = raw
    end
    Input-->>System: ESP_OK
```

## ShortPress Sequence

`ShortPress` is not emitted immediately on release. The service waits for the double-press window. If a second press does not arrive, it emits the short press.

```mermaid
sequenceDiagram
    autonumber
    participant User as User
    participant GPIO as GPIO
    participant Input as InputService
    participant System as System
    participant App as Current app

    User->>GPIO: press button
    System->>Input: update(now_ms)
    Input->>GPIO: gpio_get_level()
    GPIO-->>Input: pressed
    Input->>Input: wait kDebounceMs
    Input->>Input: stable_pressed = true
    Input->>Input: push(Pressed)
    User->>GPIO: release before kLongPressMs
    System->>Input: update(now_ms)
    Input->>Input: stable_pressed = false
    Input->>Input: push(Released)
    Input->>Input: waiting_single = true
    Note over Input: ShortPress is not emitted yet
    System->>Input: update(now_ms > pending + kDoublePressMs)
    Input->>Input: push(ShortPress)
    System->>Input: poll(event)
    Input-->>System: ShortPress
    System->>App: handleInput(event)
```

## DoublePress Sequence

```mermaid
sequenceDiagram
    autonumber
    participant User as User
    participant Input as InputService
    participant System as System
    participant App as Current app

    User->>Input: first short press
    Input->>Input: waiting_single = true
    User->>Input: second short press before 300 ms
    Input->>Input: waiting_single = false
    Input->>Input: push(DoublePress)
    System->>Input: poll(event)
    Input-->>System: DoublePress
    System->>App: handleInput(event)
```

## LongPress Sequence

`LongPress` is emitted while the button is still pressed, not when it is released.

```mermaid
sequenceDiagram
    autonumber
    participant User as User
    participant Input as InputService
    participant System as System
    participant App as Current app

    User->>Input: press and hold
    Input->>Input: push(Pressed)
    loop ticks
        System->>Input: update(now_ms)
        Input->>Input: now - press_start_ms >= 650
    end
    Input->>Input: long_emitted = true
    Input->>Input: waiting_single = false
    Input->>Input: push(LongPress)
    System->>Input: poll(event)
    Input-->>System: LongPress
    System->>App: handleInput(event)
    User->>Input: release
    Input->>Input: push(Released)
```

## Ring Buffer Sequence

If the queue is full, the oldest event is dropped so input processing keeps moving.

```mermaid
sequenceDiagram
    autonumber
    participant Input as InputService
    participant Queue as queue_[16]
    participant System as System

    Input->>Queue: push(event)
    alt next == tail
        Input->>Queue: tail = tail + 1
        Note over Input,Queue: The oldest event is discarded
    end
    Input->>Queue: queue[head] = event
    Input->>Queue: head = next
    System->>Input: poll(event)
    Input-->>System: event or false
```

## Launcher Mapping

| Event | Behavior |
|---|---|
| `Secondary + ShortPress` | next app |
| `Secondary + LongPress` | previous app |
| `Secondary + DoublePress` | toggle Cover Flow/List |
| `Primary + ShortPress` | launch selected app |
| `Primary + LongPress` | toggle Cover Flow/List |

## Global Mapping

When the current app is not `LauncherApp`, `System` intercepts:

| Event | Behavior |
|---|---|
| `Primary + LongPress` | return to launcher |

```mermaid
sequenceDiagram
    autonumber
    participant Input as InputService
    participant System as System
    participant Manager as AppManager
    participant Launcher as LauncherApp
    participant App as Current app

    Input-->>System: Primary + LongPress
    System->>Manager: current()
    alt current != LauncherApp
        System->>Manager: launch(LauncherApp)
        Manager->>App: onExit()
        Manager->>Launcher: onEnter()
    else current == LauncherApp
        System->>Manager: handleInput(event)
        Manager->>Launcher: handleInput(event)
    end
```

## Future Cardputer-Adv Mapping

The model already includes inputs that do not exist on M5StickC Plus:

| Future input | Proposed key |
|---|---|
| arrow keys | `Left`, `Right`, `Up`, `Down` |
| Enter | `Primary` |
| Esc or Backspace | `Back` |
| dedicated Home key or shortcut | `Home` |
| printable keys | `Character` with `character` set |

```mermaid
flowchart LR
    Keyboard[Cardputer keyboard HAL] --> InputService[InputService]
    Buttons[M5StickC Plus buttons] --> InputService
    InputService --> Events[Normalized InputEvent]
    Events --> Launcher[LauncherApp]
    Events --> Apps[Native apps]
```
