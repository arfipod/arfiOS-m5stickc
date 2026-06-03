#pragma once

#include "arfi/core/App.hpp"
#include "arfi/ui/Theme.hpp"

namespace arfi {

class SettingsApp final : public App {
public:
    explicit SettingsApp(SystemContext& ctx) : App(ctx) {}

    const char* id() const override { return "settings"; }
    const char* name() const override { return "Settings"; }

    void onEnter() override;
    void onExit() override;
    void update(uint32_t dt_ms) override;
    void render(Canvas& canvas) override;
    bool handleInput(const InputEvent& event) override;

private:
    void changeSelected();

    Theme theme_ = defaultTheme();
    uint8_t selected_row_ = 0;
    int brightness_ = 90;
    int view_mode_ = 0;
};

} // namespace arfi
