#include "arfi/services/PowerService.hpp"

namespace arfi {

esp_err_t PowerService::begin(const BoardConfig& board) {
    board_ = board;
    if (board.type == BoardType::M5StickCPlus) {
        return m5_power_.begin(board);
    }
    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t PowerService::setBacklightPercent(uint8_t percent) {
    if (percent > 100) {
        percent = 100;
    }
    backlight_percent_ = percent;

    if (board_.type == BoardType::M5StickCPlus) {
        return m5_power_.setBacklightPercent(percent);
    }
    return ESP_ERR_NOT_SUPPORTED;
}

} // namespace arfi
