#include "arfi/apps/SettingsApp.hpp"
#include "arfi/core/SystemContext.hpp"
#include "arfi/services/PowerService.hpp"
#include "arfi/services/SettingsService.hpp"
#include "arfi/ui/Canvas.hpp"

#include <cstdio>

namespace arfi {

void SettingsApp::onEnter() {
    brightness_ = ctx_.settings->getInt("ui_bright", 90);
    view_mode_ = ctx_.settings->getInt("lnch_view", 0);
}

void SettingsApp::onExit() {
    ctx_.settings->setInt("ui_bright", brightness_);
    ctx_.settings->setInt("lnch_view", view_mode_);
}

void SettingsApp::update(uint32_t) {}

void SettingsApp::render(Canvas& canvas) {
    canvas.clear(theme_.background);
    canvas.drawText(6, 4, "SETTINGS", theme_.text, 1);
    canvas.hline(0, 16, canvas.width(), theme_.surface_alt);

    char line[48];

    for (uint8_t row = 0; row < 2; ++row) {
        const int y = 32 + row * 28;
        const bool selected = selected_row_ == row;
        if (selected) {
            canvas.fillRect(4, y - 4, canvas.width() - 8, 22, theme_.accent);
        }

        if (row == 0) {
            std::snprintf(line, sizeof(line), "BRIGHTNESS %d%%", brightness_);
        } else {
            std::snprintf(line, sizeof(line), "LAUNCHER %s", view_mode_ == 0 ? "COVER" : "LIST");
        }

        canvas.drawText(12, y, line, selected ? Colors::Black : theme_.text, 1);
    }

    canvas.drawText(8, 104, "B NEXT  A CHANGE", theme_.muted, 1);
    canvas.drawText(8, 118, "A LONG HOME", theme_.muted, 1);
}

bool SettingsApp::handleInput(const InputEvent& event) {
    if (event.key == Key::Secondary && event.type == InputEventType::ShortPress) {
        selected_row_ = static_cast<uint8_t>((selected_row_ + 1) % 2);
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::LongPress) {
        selected_row_ = selected_row_ == 0 ? 1 : 0;
        return true;
    }

    if (event.key == Key::Primary && event.type == InputEventType::ShortPress) {
        changeSelected();
        return true;
    }

    return false;
}

void SettingsApp::changeSelected() {
    if (selected_row_ == 0) {
        brightness_ += 10;
        if (brightness_ > 100) {
            brightness_ = 20;
        }
        ctx_.settings->setInt("ui_bright", brightness_);
        ctx_.power->setBacklightPercent(static_cast<uint8_t>(brightness_));
    } else {
        view_mode_ = view_mode_ == 0 ? 1 : 0;
        ctx_.settings->setInt("lnch_view", view_mode_);
    }
}

} // namespace arfi
