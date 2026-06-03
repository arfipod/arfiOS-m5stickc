#pragma once

#include "arfi/core/App.hpp"
#include "arfi/ui/CoverFlowRenderer.hpp"
#include "arfi/ui/ListRenderer.hpp"
#include "arfi/ui/Theme.hpp"

namespace arfi {

enum class LauncherViewMode : uint8_t {
    CoverFlow = 0,
    List = 1,
};

class LauncherApp final : public App {
public:
    explicit LauncherApp(SystemContext& ctx) : App(ctx) {}

    const char* id() const override { return "launcher"; }
    const char* name() const override { return "Launcher"; }

    void onEnter() override;
    void onExit() override;
    void update(uint32_t dt_ms) override;
    void render(Canvas& canvas) override;
    bool handleInput(const InputEvent& event) override;

private:
    void next();
    void previous();
    void launchSelected();
    void toggleViewMode();
    void clampSelected();

    Theme theme_ = defaultTheme();
    CoverFlowRenderer cover_flow_;
    ListRenderer list_;
    size_t selected_index_ = 0;
    LauncherViewMode view_mode_ = LauncherViewMode::CoverFlow;
};

} // namespace arfi
