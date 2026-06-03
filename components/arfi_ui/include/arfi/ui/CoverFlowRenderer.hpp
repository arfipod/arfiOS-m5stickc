#pragma once

#include "arfi/core/AppRegistry.hpp"
#include "arfi/ui/Canvas.hpp"
#include "arfi/ui/Theme.hpp"

namespace arfi {

class CoverFlowRenderer {
public:
    void render(Canvas& canvas, const AppRegistry& registry, size_t selected_index, const Theme& theme);

private:
    void drawCard(Canvas& canvas, const AppDescriptor& app, int x, int y, int w, int h, bool selected, const Theme& theme);
};

} // namespace arfi
