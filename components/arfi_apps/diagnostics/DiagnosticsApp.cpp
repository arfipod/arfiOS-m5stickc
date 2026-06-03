#include "arfi/apps/DiagnosticsApp.hpp"
#include "arfi/core/SystemContext.hpp"
#include "arfi/ui/Canvas.hpp"

#include "esp_heap_caps.h"

#include <cstdio>

namespace arfi {

void DiagnosticsApp::update(uint32_t dt_ms) { uptime_ms_ += dt_ms; }

void DiagnosticsApp::render(Canvas& canvas) {
    canvas.clear(theme_.background);
    canvas.drawText(6, 4, "DIAGNOSTICS", theme_.text, 1);
    canvas.hline(0, 16, canvas.width(), theme_.surface_alt);

    char line[64];
    std::snprintf(line, sizeof(line), "BOARD %s", ctx_.board.name);
    canvas.drawText(8, 28, line, theme_.text, 1);

    std::snprintf(line, sizeof(line), "UPTIME %luS", static_cast<unsigned long>(uptime_ms_ / 1000));
    canvas.drawText(8, 44, line, theme_.text, 1);

    std::snprintf(line, sizeof(line), "HEAP %lu", static_cast<unsigned long>(heap_caps_get_free_size(MALLOC_CAP_INTERNAL)));
    canvas.drawText(8, 60, line, theme_.text, 1);

    std::snprintf(line, sizeof(line), "INPUT %s", last_event_);
    canvas.drawText(8, 76, line, theme_.text, 1);

    canvas.drawText(8, 108, "PRESS BUTTONS TO TEST", theme_.muted, 1);
    canvas.drawText(8, 122, "A LONG HOME", theme_.muted, 1);
}

bool DiagnosticsApp::handleInput(const InputEvent& event) {
    if (event.key == Key::Primary && event.type == InputEventType::ShortPress) {
        last_event_ = "A SHORT";
        return true;
    }
    if (event.key == Key::Secondary && event.type == InputEventType::ShortPress) {
        last_event_ = "B SHORT";
        return true;
    }
    if (event.key == Key::Secondary && event.type == InputEventType::LongPress) {
        last_event_ = "B LONG";
        return true;
    }
    if (event.type == InputEventType::DoublePress) {
        last_event_ = "DOUBLE";
        return true;
    }
    return false;
}

} // namespace arfi
