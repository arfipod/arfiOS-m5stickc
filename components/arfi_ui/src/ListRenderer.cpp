#include "arfi/ui/ListRenderer.hpp"

namespace arfi {

void ListRenderer::render(Canvas& canvas, const AppRegistry& registry, size_t selected_index, const Theme& theme) {
    canvas.clear(theme.background);
    canvas.drawText(6, 4, "ARFIOS APPS", theme.text, 1);
    canvas.hline(0, 16, canvas.width(), theme.surface_alt);

    if (registry.count() == 0) {
        canvas.drawCenteredText(canvas.width() / 2, 60, "NO APPS", theme.warning, 2);
        return;
    }

    const int row_h = 24;
    const int visible_rows = 4;
    size_t start = 0;
    if (selected_index >= 2) {
        start = selected_index - 2;
    }
    if (start + visible_rows > registry.count()) {
        start = registry.count() > visible_rows ? registry.count() - visible_rows : 0;
    }

    for (int row = 0; row < visible_rows; ++row) {
        const size_t idx = start + row;
        if (idx >= registry.count()) {
            break;
        }

        const AppDescriptor* app = registry.at(idx);
        if (app == nullptr) {
            continue;
        }

        const int y = 22 + row * row_h;
        const bool selected = idx == selected_index;
        if (selected) {
            canvas.fillRect(4, y - 3, canvas.width() - 8, row_h - 2, theme.accent);
            canvas.drawText(10, y + 2, ">", Colors::Black, 1);
            canvas.drawText(24, y + 2, app->name, Colors::Black, 1);
            canvas.drawText(184, y + 2, app->glyph, Colors::Black, 1);
        } else {
            canvas.drawText(24, y + 2, app->name, theme.text, 1);
            canvas.drawText(184, y + 2, app->glyph, theme.muted, 1);
        }
    }

    canvas.drawCenteredText(canvas.width() / 2, 122, "A OPEN  A LONG COVER", theme.muted, 1);
}

} // namespace arfi
