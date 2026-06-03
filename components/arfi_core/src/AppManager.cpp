#include "arfi/core/AppManager.hpp"

namespace arfi {

void AppManager::launch(App* next) {
    if (next == nullptr || next == current_) {
        return;
    }

    if (current_ != nullptr) {
        current_->onExit();
    }

    current_ = next;
    current_->onEnter();
}

void AppManager::update(uint32_t dt_ms) {
    if (current_ != nullptr) {
        current_->update(dt_ms);
    }
}

void AppManager::render(Canvas& canvas) {
    if (current_ != nullptr) {
        current_->render(canvas);
    }
}

bool AppManager::handleInput(const InputEvent& event) {
    if (current_ == nullptr) {
        return false;
    }

    return current_->handleInput(event);
}

} // namespace arfi
