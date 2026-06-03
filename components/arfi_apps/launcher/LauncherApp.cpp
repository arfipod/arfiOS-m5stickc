#include "arfi/apps/LauncherApp.hpp"
#include "arfi/core/AppManager.hpp"
#include "arfi/core/AppRegistry.hpp"
#include "arfi/core/SystemContext.hpp"
#include "arfi/services/SettingsService.hpp"
#include "arfi/ui/Canvas.hpp"

namespace arfi {

void LauncherApp::onEnter() {
    selected_index_ = static_cast<size_t>(ctx_.settings->getInt("lnch_sel", 0));
    view_mode_ = static_cast<LauncherViewMode>(ctx_.settings->getInt("lnch_view", 0));
    clampSelected();
}

void LauncherApp::onExit() {
    ctx_.settings->setInt("lnch_sel", static_cast<int32_t>(selected_index_));
    ctx_.settings->setInt("lnch_view", static_cast<int32_t>(view_mode_));
}

void LauncherApp::update(uint32_t) {}

void LauncherApp::render(Canvas& canvas) {
    if (view_mode_ == LauncherViewMode::CoverFlow) {
        cover_flow_.render(canvas, *ctx_.app_registry, selected_index_, theme_);
    } else {
        list_.render(canvas, *ctx_.app_registry, selected_index_, theme_);
    }
}

bool LauncherApp::handleInput(const InputEvent& event) {
    if (event.key == Key::Secondary && event.type == InputEventType::ShortPress) {
        next();
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::LongPress) {
        previous();
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::DoublePress) {
        toggleViewMode();
        return true;
    }

    if (event.key == Key::Primary && event.type == InputEventType::ShortPress) {
        launchSelected();
        return true;
    }

    if (event.key == Key::Primary && event.type == InputEventType::LongPress) {
        toggleViewMode();
        return true;
    }

    return false;
}

void LauncherApp::next() {
    if (ctx_.app_registry->count() == 0) {
        return;
    }
    selected_index_ = (selected_index_ + 1) % ctx_.app_registry->count();
}

void LauncherApp::previous() {
    if (ctx_.app_registry->count() == 0) {
        return;
    }
    if (selected_index_ == 0) {
        selected_index_ = ctx_.app_registry->count() - 1;
    } else {
        --selected_index_;
    }
}

void LauncherApp::launchSelected() {
    const AppDescriptor* selected = ctx_.app_registry->at(selected_index_);
    if (selected != nullptr && selected->instance != nullptr) {
        ctx_.app_manager->launch(selected->instance);
    }
}

void LauncherApp::toggleViewMode() {
    view_mode_ = view_mode_ == LauncherViewMode::CoverFlow ? LauncherViewMode::List : LauncherViewMode::CoverFlow;
    ctx_.settings->setInt("lnch_view", static_cast<int32_t>(view_mode_));
}

void LauncherApp::clampSelected() {
    if (ctx_.app_registry->count() == 0) {
        selected_index_ = 0;
        return;
    }
    if (selected_index_ >= ctx_.app_registry->count()) {
        selected_index_ = ctx_.app_registry->count() - 1;
    }
}

} // namespace arfi
