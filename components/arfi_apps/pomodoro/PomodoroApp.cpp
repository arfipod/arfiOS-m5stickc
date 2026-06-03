#include "arfi/apps/PomodoroApp.hpp"
#include "arfi/ui/Canvas.hpp"

#include <cstdio>

namespace arfi {

void PomodoroApp::onEnter() { reset(); }

void PomodoroApp::update(uint32_t dt_ms) {
    if (!running_ || remaining_seconds_ <= 0) {
        return;
    }

    accumulator_ms_ += dt_ms;
    while (accumulator_ms_ >= 1000 && remaining_seconds_ > 0) {
        accumulator_ms_ -= 1000;
        --remaining_seconds_;
    }

    if (remaining_seconds_ <= 0) {
        running_ = false;
    }
}

void PomodoroApp::render(Canvas& canvas) {
    canvas.clear(theme_.background);
    canvas.drawText(6, 4, "POMODORO", theme_.text, 1);
    canvas.hline(0, 16, canvas.width(), theme_.surface_alt);

    const int minutes = remaining_seconds_ / 60;
    const int seconds = remaining_seconds_ % 60;

    char time_text[16];
    std::snprintf(time_text, sizeof(time_text), "%02d:%02d", minutes, seconds);
    canvas.drawCenteredText(canvas.width() / 2, 44, time_text, remaining_seconds_ == 0 ? theme_.warning : theme_.text, 3);

    canvas.drawCenteredText(canvas.width() / 2, 82, running_ ? "RUNNING" : "PAUSED", running_ ? theme_.accent : theme_.muted, 1);
    canvas.drawText(8, 108, "A START/PAUSE", theme_.muted, 1);
    canvas.drawText(8, 122, "B RESET  B LONG +1M", theme_.muted, 1);
}

bool PomodoroApp::handleInput(const InputEvent& event) {
    if (event.key == Key::Primary && event.type == InputEventType::ShortPress) {
        toggle();
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::ShortPress) {
        reset();
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::LongPress) {
        remaining_seconds_ += 60;
        return true;
    }

    return false;
}

void PomodoroApp::reset() {
    running_ = false;
    remaining_seconds_ = 25 * 60;
    accumulator_ms_ = 0;
}

void PomodoroApp::toggle() {
    if (remaining_seconds_ <= 0) {
        reset();
    }
    running_ = !running_;
}

} // namespace arfi
