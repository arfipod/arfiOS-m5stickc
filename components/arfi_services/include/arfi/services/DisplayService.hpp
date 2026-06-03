#pragma once

#include "arfi/hal/BoardConfig.hpp"
#include "arfi/hal/M5StickCPlusDisplay.hpp"
#include "arfi/services/PowerService.hpp"
#include "arfi/ui/Canvas.hpp"

#include "esp_err.h"

#include <cstdint>

namespace arfi {

class DisplayService {
public:
    ~DisplayService();

    esp_err_t begin(const BoardConfig& board, PowerService& power);
    Canvas& canvas() { return canvas_; }
    esp_err_t flush();

    uint16_t width() const { return width_; }
    uint16_t height() const { return height_; }

private:
    BoardConfig board_;
    M5StickCPlusDisplay m5_display_;
    uint16_t* buffer_ = nullptr;
    uint16_t width_ = 0;
    uint16_t height_ = 0;
    Canvas canvas_{0, 0, nullptr};
};

} // namespace arfi
