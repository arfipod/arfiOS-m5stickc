#include "arfi/ui/Canvas.hpp"

#include <algorithm>
#include <array>
#include <cctype>
#include <cstring>

namespace arfi {

Canvas::Canvas(uint16_t width, uint16_t height, uint16_t* pixels) : width_(width), height_(height), pixels_(pixels) {}

void Canvas::clear(Color color) { fillRect(0, 0, width_, height_, color); }

void Canvas::pixel(int x, int y, Color color) {
    if (pixels_ == nullptr || x < 0 || y < 0 || x >= width_ || y >= height_) {
        return;
    }
    pixels_[y * width_ + x] = color.value;
}

void Canvas::fillRect(int x, int y, int w, int h, Color color) {
    if (pixels_ == nullptr || w <= 0 || h <= 0) {
        return;
    }

    const int x0 = std::max(0, x);
    const int y0 = std::max(0, y);
    const int x1 = std::min<int>(width_, x + w);
    const int y1 = std::min<int>(height_, y + h);

    if (x0 >= x1 || y0 >= y1) {
        return;
    }

    for (int yy = y0; yy < y1; ++yy) {
        uint16_t* row = pixels_ + yy * width_;
        for (int xx = x0; xx < x1; ++xx) {
            row[xx] = color.value;
        }
    }
}

void Canvas::drawRect(int x, int y, int w, int h, Color color) {
    if (w <= 0 || h <= 0) {
        return;
    }
    hline(x, y, w, color);
    hline(x, y + h - 1, w, color);
    vline(x, y, h, color);
    vline(x + w - 1, y, h, color);
}

void Canvas::hline(int x, int y, int w, Color color) { fillRect(x, y, w, 1, color); }
void Canvas::vline(int x, int y, int h, Color color) { fillRect(x, y, 1, h, color); }

int Canvas::textWidth(std::string_view text, uint8_t scale) const {
    if (text.empty()) {
        return 0;
    }
    return static_cast<int>(text.size()) * 6 * scale - scale;
}

void Canvas::drawText(int x, int y, std::string_view text, Color color, uint8_t scale) {
    int cursor = x;
    for (char c : text) {
        drawChar(cursor, y, c, color, scale);
        cursor += 6 * scale;
    }
}

void Canvas::drawCenteredText(int cx, int y, std::string_view text, Color color, uint8_t scale) {
    drawText(cx - textWidth(text, scale) / 2, y, text, color, scale);
}

bool Canvas::drawAppIcon(const AppIcon& icon, int x, int y, uint8_t scale) {
    if (!icon.valid() || scale == 0) {
        return false;
    }

    switch (icon.format) {
    case AppIconFormat::Rgb565: {
        const auto* pixels = static_cast<const uint16_t*>(icon.data);
        for (uint8_t yy = 0; yy < icon.height; ++yy) {
            for (uint8_t xx = 0; xx < icon.width; ++xx) {
                const uint16_t value = pixels[yy * icon.width + xx];
                if (icon.has_transparency && value == icon.transparent_rgb565) {
                    continue;
                }
                fillRect(x + xx * scale, y + yy * scale, scale, scale, Color(value));
            }
        }
        return true;
    }
    case AppIconFormat::Mono1: {
        const auto* bits = static_cast<const uint8_t*>(icon.data);
        for (uint8_t yy = 0; yy < icon.height; ++yy) {
            for (uint8_t xx = 0; xx < icon.width; ++xx) {
                const uint16_t bit_index = yy * icon.width + xx;
                const bool on = (bits[bit_index / 8] & (0x80 >> (bit_index % 8))) != 0;
                if (on) {
                    fillRect(x + xx * scale, y + yy * scale, scale, scale, Color(icon.foreground_rgb565));
                } else if (!icon.has_transparency) {
                    fillRect(x + xx * scale, y + yy * scale, scale, scale, Color(icon.background_rgb565));
                }
            }
        }
        return true;
    }
    }

    return false;
}

void Canvas::drawChar(int x, int y, char c, Color color, uint8_t scale) {
    const uint8_t* glyph = glyphFor(c);
    for (int col = 0; col < 5; ++col) {
        uint8_t bits = glyph[col];
        for (int row = 0; row < 7; ++row) {
            if (bits & (1 << row)) {
                fillRect(x + col * scale, y + row * scale, scale, scale, color);
            }
        }
    }
}

const uint8_t* Canvas::glyphFor(char c) {
    static const uint8_t blank[5] = {0, 0, 0, 0, 0};
    static const uint8_t box[5] = {0x7F, 0x41, 0x41, 0x41, 0x7F};

    static const uint8_t digits[10][5] = {
        {0x3E, 0x51, 0x49, 0x45, 0x3E}, // 0
        {0x00, 0x42, 0x7F, 0x40, 0x00}, // 1
        {0x42, 0x61, 0x51, 0x49, 0x46}, // 2
        {0x21, 0x41, 0x45, 0x4B, 0x31}, // 3
        {0x18, 0x14, 0x12, 0x7F, 0x10}, // 4
        {0x27, 0x45, 0x45, 0x45, 0x39}, // 5
        {0x3C, 0x4A, 0x49, 0x49, 0x30}, // 6
        {0x01, 0x71, 0x09, 0x05, 0x03}, // 7
        {0x36, 0x49, 0x49, 0x49, 0x36}, // 8
        {0x06, 0x49, 0x49, 0x29, 0x1E}, // 9
    };

    static const uint8_t letters[26][5] = {
        {0x7E, 0x11, 0x11, 0x11, 0x7E}, // A
        {0x7F, 0x49, 0x49, 0x49, 0x36}, // B
        {0x3E, 0x41, 0x41, 0x41, 0x22}, // C
        {0x7F, 0x41, 0x41, 0x22, 0x1C}, // D
        {0x7F, 0x49, 0x49, 0x49, 0x41}, // E
        {0x7F, 0x09, 0x09, 0x09, 0x01}, // F
        {0x3E, 0x41, 0x49, 0x49, 0x7A}, // G
        {0x7F, 0x08, 0x08, 0x08, 0x7F}, // H
        {0x00, 0x41, 0x7F, 0x41, 0x00}, // I
        {0x20, 0x40, 0x41, 0x3F, 0x01}, // J
        {0x7F, 0x08, 0x14, 0x22, 0x41}, // K
        {0x7F, 0x40, 0x40, 0x40, 0x40}, // L
        {0x7F, 0x02, 0x0C, 0x02, 0x7F}, // M
        {0x7F, 0x04, 0x08, 0x10, 0x7F}, // N
        {0x3E, 0x41, 0x41, 0x41, 0x3E}, // O
        {0x7F, 0x09, 0x09, 0x09, 0x06}, // P
        {0x3E, 0x41, 0x51, 0x21, 0x5E}, // Q
        {0x7F, 0x09, 0x19, 0x29, 0x46}, // R
        {0x46, 0x49, 0x49, 0x49, 0x31}, // S
        {0x01, 0x01, 0x7F, 0x01, 0x01}, // T
        {0x3F, 0x40, 0x40, 0x40, 0x3F}, // U
        {0x1F, 0x20, 0x40, 0x20, 0x1F}, // V
        {0x3F, 0x40, 0x38, 0x40, 0x3F}, // W
        {0x63, 0x14, 0x08, 0x14, 0x63}, // X
        {0x07, 0x08, 0x70, 0x08, 0x07}, // Y
        {0x61, 0x51, 0x49, 0x45, 0x43}, // Z
    };

    static const uint8_t colon[5] = {0x00, 0x36, 0x36, 0x00, 0x00};
    static const uint8_t dash[5] = {0x08, 0x08, 0x08, 0x08, 0x08};
    static const uint8_t dot[5] = {0x00, 0x60, 0x60, 0x00, 0x00};
    static const uint8_t slash[5] = {0x20, 0x10, 0x08, 0x04, 0x02};
    static const uint8_t percent[5] = {0x23, 0x13, 0x08, 0x64, 0x62};
    static const uint8_t plus[5] = {0x08, 0x08, 0x3E, 0x08, 0x08};
    static const uint8_t greater[5] = {0x00, 0x41, 0x22, 0x14, 0x08};
    static const uint8_t less[5] = {0x00, 0x08, 0x14, 0x22, 0x41};

    if (c == ' ') {
        return blank;
    }
    if (c >= '0' && c <= '9') {
        return digits[c - '0'];
    }

    c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    if (c >= 'A' && c <= 'Z') {
        return letters[c - 'A'];
    }

    switch (c) {
    case ':': return colon;
    case '-': return dash;
    case '.': return dot;
    case '/': return slash;
    case '%': return percent;
    case '+': return plus;
    case '>': return greater;
    case '<': return less;
    default: return box;
    }
}

} // namespace arfi
