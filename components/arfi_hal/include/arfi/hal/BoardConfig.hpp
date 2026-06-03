#pragma once

#include <cstdint>

namespace arfi {

enum class BoardType : uint8_t {
    M5StickCPlus,
    CardputerAdv,
    Unknown,
};

struct BoardConfig {
    BoardType type = BoardType::Unknown;
    const char* name = "Unknown";
    uint16_t display_width = 0;
    uint16_t display_height = 0;
    bool has_keyboard = false;
    bool has_micro_sd = false;
    bool has_audio_codec = false;
    bool has_imu = false;
    bool has_ir = false;
};

inline BoardConfig makeM5StickCPlusBoardConfig() {
    BoardConfig config;
    config.type = BoardType::M5StickCPlus;
    config.name = "M5StickC Plus";
    config.display_width = 240;
    config.display_height = 135;
    config.has_keyboard = false;
    config.has_micro_sd = false;
    config.has_audio_codec = false;
    config.has_imu = true;
    config.has_ir = true;
    return config;
}

} // namespace arfi
