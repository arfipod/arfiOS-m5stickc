#pragma once

#include <cstdint>

namespace arfi {

struct Color {
    uint16_t value = 0;

    constexpr Color() = default;
    constexpr explicit Color(uint16_t rgb565) : value(rgb565) {}

    static constexpr Color rgb(uint8_t r, uint8_t g, uint8_t b) {
        return Color(static_cast<uint16_t>(((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)));
    }
};

namespace Colors {
static constexpr Color Black = Color(0x0000);
static constexpr Color White = Color(0xFFFF);
static constexpr Color Red = Color(0xF800);
static constexpr Color Green = Color(0x07E0);
static constexpr Color Blue = Color(0x001F);
static constexpr Color Yellow = Color(0xFFE0);
static constexpr Color Cyan = Color(0x07FF);
static constexpr Color Magenta = Color(0xF81F);
static constexpr Color Gray10 = Color(0x1082);
static constexpr Color Gray20 = Color(0x2104);
static constexpr Color Gray30 = Color(0x3186);
static constexpr Color Gray50 = Color(0x7BEF);
static constexpr Color Orange = Color(0xFC00);
static constexpr Color Purple = Color(0x8010);
static constexpr Color DarkBlue = Color(0x0008);
static constexpr Color Accent = Color(0x7D7C);
} // namespace Colors

} // namespace arfi
