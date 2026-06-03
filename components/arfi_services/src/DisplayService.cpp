#include "arfi/services/DisplayService.hpp"

#include "esp_heap_caps.h"
#include "esp_log.h"

namespace arfi {

static const char* TAG = "display_service";

DisplayService::~DisplayService() {
    if (buffer_ != nullptr) {
        heap_caps_free(buffer_);
        buffer_ = nullptr;
    }
}

esp_err_t DisplayService::begin(const BoardConfig& board, PowerService& power) {
    board_ = board;
    width_ = board.display_width;
    height_ = board.display_height;

    const size_t buffer_size = static_cast<size_t>(width_) * height_ * sizeof(uint16_t);
    buffer_ = static_cast<uint16_t*>(heap_caps_malloc(buffer_size, MALLOC_CAP_DMA | MALLOC_CAP_INTERNAL));
    if (buffer_ == nullptr) {
        ESP_LOGE(TAG, "Failed to allocate display buffer");
        return ESP_ERR_NO_MEM;
    }

    canvas_ = Canvas(width_, height_, buffer_);
    canvas_.clear(Colors::Black);

    if (board.type == BoardType::M5StickCPlus) {
        return m5_display_.begin(board, power.m5StickCPlusPower());
    }

    return ESP_ERR_NOT_SUPPORTED;
}

esp_err_t DisplayService::flush() {
    if (board_.type == BoardType::M5StickCPlus) {
        return m5_display_.flush(buffer_, width_, height_);
    }
    return ESP_ERR_NOT_SUPPORTED;
}

} // namespace arfi
