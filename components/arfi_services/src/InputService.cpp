#include "arfi/services/InputService.hpp"
#include "arfi/hal/M5StickCPlusPins.hpp"

#include "esp_check.h"
#include "esp_log.h"

namespace arfi {

static const char* TAG = "input_service";

esp_err_t InputService::begin(const BoardConfig& board) {
    if (board.type != BoardType::M5StickCPlus) {
        return ESP_ERR_NOT_SUPPORTED;
    }

    buttons_[0].gpio = m5stickcplus::kButtonA;
    buttons_[0].key = Key::Primary;
    buttons_[0].active_low = true;

    buttons_[1].gpio = m5stickcplus::kButtonB;
    buttons_[1].key = Key::Secondary;
    buttons_[1].active_low = true;

    for (auto& button : buttons_) {
        gpio_config_t cfg = {};
        cfg.pin_bit_mask = 1ULL << button.gpio;
        cfg.mode = GPIO_MODE_INPUT;
        cfg.pull_up_en = GPIO_PULLUP_DISABLE;
        cfg.pull_down_en = GPIO_PULLDOWN_DISABLE;
        cfg.intr_type = GPIO_INTR_DISABLE;
        ESP_RETURN_ON_ERROR(gpio_config(&cfg), TAG, "button gpio config failed");

        button.stable_pressed = readPressed_(button);
        button.last_raw_pressed = button.stable_pressed;
    }

    ESP_LOGI(TAG, "Input service initialized");
    return ESP_OK;
}

void InputService::update(uint32_t now_ms) {
    for (auto& button : buttons_) {
        updateButton_(button, now_ms);
    }
}

bool InputService::poll(InputEvent& event) {
    if (queue_head_ == queue_tail_) {
        return false;
    }

    event = queue_[queue_tail_];
    queue_tail_ = (queue_tail_ + 1) % kEventQueueSize;
    return true;
}

void InputService::updateButton_(ButtonState& button, uint32_t now_ms) {
    const bool raw_pressed = readPressed_(button);

    if (raw_pressed != button.last_raw_pressed) {
        button.last_raw_pressed = raw_pressed;
        button.last_raw_change_ms = now_ms;
    }

    if ((now_ms - button.last_raw_change_ms) < kDebounceMs) {
        return;
    }

    if (raw_pressed != button.stable_pressed) {
        button.stable_pressed = raw_pressed;

        if (button.stable_pressed) {
            button.press_start_ms = now_ms;
            button.long_emitted = false;
            push_({button.key, InputEventType::Pressed, 0, now_ms});
        } else {
            push_({button.key, InputEventType::Released, 0, now_ms});
            const uint32_t held_ms = now_ms - button.press_start_ms;
            if (!button.long_emitted && held_ms < kLongPressMs) {
                if (button.waiting_single && (now_ms - button.pending_single_ms) <= kDoublePressMs) {
                    button.waiting_single = false;
                    push_({button.key, InputEventType::DoublePress, 0, now_ms});
                } else {
                    button.waiting_single = true;
                    button.pending_single_ms = now_ms;
                }
            }
        }
    }

    if (button.stable_pressed && !button.long_emitted && (now_ms - button.press_start_ms) >= kLongPressMs) {
        button.long_emitted = true;
        button.waiting_single = false;
        push_({button.key, InputEventType::LongPress, 0, now_ms});
    }

    if (button.waiting_single && (now_ms - button.pending_single_ms) > kDoublePressMs) {
        button.waiting_single = false;
        push_({button.key, InputEventType::ShortPress, 0, now_ms});
    }
}

bool InputService::readPressed_(const ButtonState& button) const {
    const int level = gpio_get_level(button.gpio);
    return button.active_low ? (level == 0) : (level != 0);
}

void InputService::push_(const InputEvent& event) {
    const size_t next = (queue_head_ + 1) % kEventQueueSize;
    if (next == queue_tail_) {
        // Drop oldest event to keep the input queue moving.
        queue_tail_ = (queue_tail_ + 1) % kEventQueueSize;
    }

    queue_[queue_head_] = event;
    queue_head_ = next;
}

} // namespace arfi
