#pragma once

#include <cstdint>

#ifndef ARFI_PROGMEM
#if defined(__AVR__) || defined(ARDUINO_ARCH_AVR)
#include <avr/pgmspace.h>
#define ARFI_PROGMEM PROGMEM
#else
#define ARFI_PROGMEM
#endif
#endif

namespace arfi {

enum class AppIconFormat : uint8_t {
    Rgb565,
    Mono1,
};

struct AppIcon {
    uint8_t width = 0;
    uint8_t height = 0;
    AppIconFormat format = AppIconFormat::Rgb565;
    const void* data = nullptr;
    uint16_t foreground_rgb565 = 0xFFFF;
    uint16_t background_rgb565 = 0x0000;
    bool has_transparency = false;
    uint16_t transparent_rgb565 = 0x0000;

    constexpr bool valid() const { return data != nullptr && width > 0 && height > 0; }
};

} // namespace arfi
