#pragma once

#include "esp_err.h"
#include "esp_lcd_panel_ops.h"
#include "esp_lcd_panel_io.h"
#include "arfi/hal/BoardConfig.hpp"

#include <cstdint>

namespace arfi {

class M5StickCPlusPower;

class M5StickCPlusDisplay {
public:
    esp_err_t begin(const BoardConfig& board, M5StickCPlusPower& power);
    esp_err_t flush(const uint16_t* pixels, uint16_t width, uint16_t height);

private:
    esp_lcd_panel_io_handle_t io_ = nullptr;
    esp_lcd_panel_handle_t panel_ = nullptr;
    bool initialized_ = false;
};

} // namespace arfi
