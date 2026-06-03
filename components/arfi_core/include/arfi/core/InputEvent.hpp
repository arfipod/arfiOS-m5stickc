#pragma once

#include <cstdint>

namespace arfi {

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
    Key key = Key::Unknown;
    InputEventType type = InputEventType::Pressed;
    char character = 0;
    uint32_t timestamp_ms = 0;

    bool isShortPress() const { return type == InputEventType::ShortPress; }
    bool isLongPress() const { return type == InputEventType::LongPress; }
    bool isDoublePress() const { return type == InputEventType::DoublePress; }
};

} // namespace arfi
