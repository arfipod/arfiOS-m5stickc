#include "arfi/ui/CoverFlowRenderer.hpp"

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

bool drawCenteredAppIcon(Canvas& canvas, const AppIcon* icon, int cx, int cy, int max_w, int max_h, uint8_t max_scale) {
    if (icon == nullptr || !icon->valid()) {
        return false;
    }

    const uint8_t scale = fitIconScale(*icon, max_w, max_h, max_scale);
    if (scale == 0) {
        return false;
    }

    const int draw_w = icon->width * scale;
    const int draw_h = icon->height * scale;
    return canvas.drawAppIcon(*icon, cx - draw_w / 2, cy - draw_h / 2, scale);
}

} // namespace

void CoverFlowRenderer::render(Canvas& canvas, const AppRegistry& registry, size_t selected_index, const Theme& theme) {
    canvas.clear(theme.background);

    canvas.drawCenteredText(canvas.width() / 2, 4, "ARFIOS", theme.text, 1);
    canvas.hline(0, 16, canvas.width(), theme.surface_alt);

    if (registry.count() == 0) {
        canvas.drawCenteredText(canvas.width() / 2, 60, "NO APPS", theme.warning, 2);
        return;
    }

    const int center_x = canvas.width() / 2;
    const int center_y = 64;

    for (int delta = -2; delta <= 2; ++delta) {
        const int idx = static_cast<int>(selected_index) + delta;
        if (idx < 0 || idx >= static_cast<int>(registry.count())) {
            continue;
        }

        const AppDescriptor* app = registry.at(static_cast<size_t>(idx));
        if (app == nullptr) {
            continue;
        }

        const bool selected = delta == 0;
        const int w = selected ? 78 : 42;
        const int h = selected ? 58 : 34;
        const int x = center_x - w / 2 + delta * 50;
        const int y = center_y - h / 2 + (selected ? 0 : 9);
        drawCard(canvas, *app, x, y, w, h, selected, theme);
    }

    const AppDescriptor* selected = registry.at(selected_index);
    if (selected != nullptr) {
        canvas.drawCenteredText(canvas.width() / 2, 102, selected->name, theme.text, 1);
        canvas.drawCenteredText(canvas.width() / 2, 116, "B NEXT  A OPEN", theme.muted, 1);
    }
}

void CoverFlowRenderer::drawCard(
    Canvas& canvas,
    const AppDescriptor& app,
    int x,
    int y,
    int w,
    int h,
    bool selected,
    const Theme& theme) {
    const Color fill = selected ? theme.accent : theme.surface;
    const Color border = selected ? theme.text : theme.surface_alt;
    const Color text = selected ? Colors::Black : theme.text;

    canvas.fillRect(x, y, w, h, fill);
    canvas.drawRect(x, y, w, h, border);

    const int max_icon_w = w - (selected ? 22 : 14);
    const int max_icon_h = h - (selected ? 22 : 12);
    const uint8_t max_scale = selected ? 4 : 2;
    if (!drawCenteredAppIcon(canvas, app.icon, x + w / 2, y + h / 2 - (selected ? 4 : 0), max_icon_w, max_icon_h, max_scale)) {
        canvas.drawCenteredText(x + w / 2, y + h / 2 - (selected ? 10 : 4), app.glyph, text, selected ? 2 : 1);
    }
}

} // namespace arfi
