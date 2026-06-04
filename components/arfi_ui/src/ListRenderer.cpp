#include "arfi/ui/ListRenderer.hpp"

#include <algorithm>

namespace arfi {

namespace {

uint8_t fitIconScale(const AppIcon& icon, int max_w, int max_h, uint8_t max_scale) {
    if (!icon.valid() || max_w <= 0 || max_h <= 0) {
        return 0;
    }

    const int scale = std::min(max_w / icon.width, max_h / icon.height);
    if (scale <= 0) {
        return 0;
    }
    return static_cast<uint8_t>(std::min<int>(scale, max_scale));
}

bool drawListIcon(Canvas& canvas, const AppIcon* icon, int right_x, int center_y) {
    if (icon == nullptr || !icon->valid()) {
        return false;
    }

    const uint8_t scale = fitIconScale(*icon, 18, 18, 2);
    if (scale == 0) {
        return false;
    }

    const int draw_w = icon->width * scale;
    const int draw_h = icon->height * scale;
    return canvas.drawAppIcon(*icon, right_x - draw_w, center_y - draw_h / 2, scale);
}

} // namespace

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
            if (!drawListIcon(canvas, app->icon, canvas.width() - 12, y + row_h / 2 - 3)) {
                canvas.drawText(184, y + 2, app->glyph, Colors::Black, 1);
            }
        } else {
            canvas.drawText(24, y + 2, app->name, theme.text, 1);
            if (!drawListIcon(canvas, app->icon, canvas.width() - 12, y + row_h / 2 - 3)) {
                canvas.drawText(184, y + 2, app->glyph, theme.muted, 1);
            }
        }
    }

    canvas.drawCenteredText(canvas.width() / 2, 122, "A OPEN  A LONG COVER", theme.muted, 1);
}

} // namespace arfi
