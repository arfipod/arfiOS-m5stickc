#pragma once

#include "arfi/ui/Color.hpp"

namespace arfi {

struct Theme {
    Color background = Colors::Black;
    Color surface = Colors::Gray10;
    Color surface_alt = Colors::Gray20;
    Color text = Colors::White;
    Color muted = Colors::Gray50;
    Color accent = Colors::Accent;
    Color warning = Colors::Orange;
};

inline Theme defaultTheme() { return Theme{}; }

} // namespace arfi
