#pragma once

#include "arfi/core/AppIcon.hpp"
#include "arfi/ui/Color.hpp"

#include <cstdint>

namespace arfi::app_icons {

inline constexpr uint8_t kSettingsBits[] ARFI_PROGMEM = {
    0b00111100,
    0b01011010,
    0b10100101,
    0b10111101,
    0b10111101,
    0b10100101,
    0b01011010,
    0b00111100,
};

inline constexpr uint8_t kDiagnosticsBits[] ARFI_PROGMEM = {
    0b00011000,
    0b00100100,
    0b01000010,
    0b11111111,
    0b01000010,
    0b00100100,
    0b00011000,
    0b00000000,
};

inline constexpr uint8_t kPomodoroBits[] ARFI_PROGMEM = {
    0b00111100,
    0b01000010,
    0b10100101,
    0b10011001,
    0b10100001,
    0b10000001,
    0b01000010,
    0b00111100,
};

inline constexpr uint8_t kImuLevelBits[] ARFI_PROGMEM = {
    0b00000000,
    0b00111100,
    0b01000010,
    0b10011001,
    0b10011001,
    0b01000010,
    0b00111100,
    0b00000000,
};

inline constexpr uint8_t kIrSweepBits[] ARFI_PROGMEM = {
    0b10000001,
    0b01000010,
    0b00100100,
    0b00011000,
    0b00011000,
    0b00100100,
    0b01000010,
    0b10000001,
};

inline constexpr uint8_t kRestBits[] ARFI_PROGMEM = {
    0b11111100,
    0b10000010,
    0b10111010,
    0b10101010,
    0b10111010,
    0b10000010,
    0b11111100,
    0b00000000,
};

inline constexpr uint8_t kCube3DBits[] ARFI_PROGMEM = {
    0b00111100,
    0b01100110,
    0b11011010,
    0b10111110,
    0b10100110,
    0b11011010,
    0b01100110,
    0b00111100,
};

inline constexpr uint8_t kAboutBits[] ARFI_PROGMEM = {
    0b00111100,
    0b01000010,
    0b10011001,
    0b10100101,
    0b10100101,
    0b10011001,
    0b01000010,
    0b00111100,
};

inline constexpr uint16_t kTransparent = Colors::Magenta.value;
inline constexpr uint16_t kYellow = Colors::Yellow.value;
inline constexpr uint16_t kOrange = Colors::Orange.value;
inline constexpr uint16_t kWhite = Colors::White.value;
inline constexpr uint16_t kBlack = Colors::Black.value;

inline constexpr uint16_t kFlappyPixels[] ARFI_PROGMEM = {
    kTransparent, kTransparent, kTransparent, kYellow,      kYellow,      kTransparent, kTransparent, kTransparent,
    kTransparent, kTransparent, kYellow,      kYellow,      kYellow,      kYellow,      kTransparent, kTransparent,
    kTransparent, kYellow,      kYellow,      kWhite,       kBlack,       kYellow,      kYellow,      kTransparent,
    kYellow,      kYellow,      kYellow,      kYellow,      kYellow,      kOrange,      kOrange,      kTransparent,
    kTransparent, kYellow,      kYellow,      kYellow,      kYellow,      kYellow,      kTransparent, kTransparent,
    kTransparent, kTransparent, kYellow,      kBlack,       kYellow,      kTransparent, kTransparent, kTransparent,
    kTransparent, kYellow,      kTransparent, kTransparent, kYellow,      kTransparent, kTransparent, kTransparent,
    kTransparent, kTransparent, kTransparent, kTransparent, kTransparent, kTransparent, kTransparent, kTransparent,
};

inline constexpr AppIcon Settings = {
    8, 8, AppIconFormat::Mono1, kSettingsBits, Colors::Cyan.value, Colors::Black.value, true, Colors::Black.value};
inline constexpr AppIcon Diagnostics = {
    8, 8, AppIconFormat::Mono1, kDiagnosticsBits, Colors::Yellow.value, Colors::Black.value, true, Colors::Black.value};
inline constexpr AppIcon Pomodoro = {
    8, 8, AppIconFormat::Mono1, kPomodoroBits, Colors::Orange.value, Colors::Black.value, true, Colors::Black.value};
inline constexpr AppIcon ImuLevel = {
    8, 8, AppIconFormat::Mono1, kImuLevelBits, Colors::Green.value, Colors::Black.value, true, Colors::Black.value};
inline constexpr AppIcon IrSweep = {
    8, 8, AppIconFormat::Mono1, kIrSweepBits, Colors::Red.value, Colors::Black.value, true, Colors::Black.value};
inline constexpr AppIcon FlappyBird = {
    8, 8, AppIconFormat::Rgb565, kFlappyPixels, Colors::White.value, Colors::Black.value, true, Colors::Magenta.value};
inline constexpr AppIcon Cube3D = {
    8, 8, AppIconFormat::Mono1, kCube3DBits, Colors::Accent.value, Colors::Black.value, true, Colors::Black.value};
inline constexpr AppIcon Rest = {
    8, 8, AppIconFormat::Mono1, kRestBits, Colors::Cyan.value, Colors::Black.value, true, Colors::Black.value};
inline constexpr AppIcon About = {
    8, 8, AppIconFormat::Mono1, kAboutBits, Colors::White.value, Colors::Black.value, true, Colors::Black.value};

} // namespace arfi::app_icons
