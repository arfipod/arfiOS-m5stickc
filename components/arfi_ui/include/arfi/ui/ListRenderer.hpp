#pragma once

#include "arfi/core/AppRegistry.hpp"
#include "arfi/ui/Canvas.hpp"
#include "arfi/ui/Theme.hpp"

namespace arfi {

class ListRenderer {
public:
    void render(Canvas& canvas, const AppRegistry& registry, size_t selected_index, const Theme& theme);
};

} // namespace arfi
