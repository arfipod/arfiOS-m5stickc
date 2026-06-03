#include "arfi/apps/AboutApp.hpp"
#include "arfi/core/SystemContext.hpp"
#include "arfi/ui/Canvas.hpp"

namespace arfi {

void AboutApp::update(uint32_t) {}

void AboutApp::render(Canvas& canvas) {
    canvas.clear(theme_.background);
    canvas.drawCenteredText(canvas.width() / 2, 16, "ARFIOS", theme_.accent, 2);
    canvas.drawCenteredText(canvas.width() / 2, 42, "VERSION 0.1.0", theme_.text, 1);
    canvas.drawCenteredText(canvas.width() / 2, 58, "ESP-IDF RUNTIME", theme_.text, 1);
    canvas.drawCenteredText(canvas.width() / 2, 74, "NOT A REAL OS", theme_.muted, 1);
    canvas.drawCenteredText(canvas.width() / 2, 94, ctx_.board.name, theme_.text, 1);
    canvas.drawCenteredText(canvas.width() / 2, 120, "A LONG HOME", theme_.muted, 1);
}

bool AboutApp::handleInput(const InputEvent&) { return false; }

} // namespace arfi
