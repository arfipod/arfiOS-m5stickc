#pragma once

#include "arfi/core/InputEvent.hpp"
#include "arfi/hal/BoardConfig.hpp"

#include "driver/gpio.h"
#include "esp_err.h"

#include <array>
#include <cstddef>
#include <cstdint>

namespace arfi {

class InputService {
public:
    esp_err_t begin(const BoardConfig& board);
    void update(uint32_t now_ms);
    bool poll(InputEvent& event);

private:
    struct ButtonState {
        gpio_num_t gpio = GPIO_NUM_NC;
        Key key = Key::Unknown;
        bool active_low = true;
        bool stable_pressed = false;
        bool last_raw_pressed = false;
        uint32_t last_raw_change_ms = 0;
        uint32_t press_start_ms = 0;
        bool long_emitted = false;
        bool waiting_single = false;
        uint32_t pending_single_ms = 0;
    };

    static constexpr uint32_t kDebounceMs = 30;
    static constexpr uint32_t kLongPressMs = 650;
    static constexpr uint32_t kDoublePressMs = 300;
    static constexpr size_t kEventQueueSize = 16;

    void updateButton_(ButtonState& button, uint32_t now_ms);
    bool readPressed_(const ButtonState& button) const;
    void push_(const InputEvent& event);

    std::array<ButtonState, 2> buttons_;
    std::array<InputEvent, kEventQueueSize> queue_;
    size_t queue_head_ = 0;
    size_t queue_tail_ = 0;
};

} // namespace arfi
