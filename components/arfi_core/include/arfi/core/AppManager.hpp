#pragma once

#include "arfi/core/App.hpp"

namespace arfi {

class AppManager {
public:
    void launch(App* next);
    void update(uint32_t dt_ms);
    void render(Canvas& canvas);
    bool handleInput(const InputEvent& event);

    App* current() const { return current_; }

private:
    App* current_ = nullptr;
};

} // namespace arfi
