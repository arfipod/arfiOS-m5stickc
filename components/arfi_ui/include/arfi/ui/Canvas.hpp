#pragma once

#include "arfi/ui/Color.hpp"

#include <cstdint>
#include <string_view>

namespace arfi {

class Canvas {
public:
    Canvas(uint16_t width, uint16_t height, uint16_t* pixels);

    uint16_t width() const { return width_; }
    uint16_t height() const { return height_; }
    uint16_t* pixels() { return pixels_; }
    const uint16_t* pixels() const { return pixels_; }

    void clear(Color color);
    void pixel(int x, int y, Color color);
    void fillRect(int x, int y, int w, int h, Color color);
    void drawRect(int x, int y, int w, int h, Color color);
    void hline(int x, int y, int w, Color color);
    void vline(int x, int y, int h, Color color);

    void drawText(int x, int y, std::string_view text, Color color, uint8_t scale = 1);
    void drawCenteredText(int cx, int y, std::string_view text, Color color, uint8_t scale = 1);
    int textWidth(std::string_view text, uint8_t scale = 1) const;

private:
    void drawChar(int x, int y, char c, Color color, uint8_t scale);
    static const uint8_t* glyphFor(char c);

    uint16_t width_ = 0;
    uint16_t height_ = 0;
    uint16_t* pixels_ = nullptr;
};

} // namespace arfi
