# arfiOS Input Model

arfiOS normalizes hardware buttons and future keyboards into common input events.

The first target, M5StickC Plus, has two regular user buttons:

- Button A on GPIO37;
- Button B on GPIO39.

Because two buttons are restrictive, arfiOS uses press semantics:

- short press;
- long press;
- double press.

## Event shape

```cpp
enum class Key {
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

enum class InputEventType {
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

## M5StickC Plus mapping

| Physical input | arfiOS key |
|---|---|
| Button A | `Primary` |
| Button B | `Secondary` |

## Launcher mapping

| Event | Behavior |
|---|---|
| `Secondary + ShortPress` | Next app |
| `Secondary + LongPress` | Previous app |
| `Primary + ShortPress` | Launch selected app |
| `Primary + LongPress` | Toggle Cover Flow/List |
| `Secondary + DoublePress` | Toggle Cover Flow/List |

## Global mapping

When the current foreground app is not the launcher:

| Event | Behavior |
|---|---|
| `Primary + LongPress` | Return to launcher |

## Future Cardputer-Adv mapping

The Cardputer-Adv target should map:

- arrow keys to `Left`, `Right`, `Up`, `Down`;
- Enter to `Primary`;
- Esc/Backspace to `Back`;
- printable keys to `Character`;
- a chosen shortcut to `Home`.
