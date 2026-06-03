#pragma once

#include "arfi/core/App.hpp"
#include "arfi/ui/Theme.hpp"

namespace arfi {

class PomodoroApp final : public App {
public:
    explicit PomodoroApp(SystemContext& ctx) : App(ctx) {}

    const char* id() const override { return "pomodoro"; }
    const char* name() const override { return "Pomodoro"; }

    void onEnter() override;
    void update(uint32_t dt_ms) override;
    void render(Canvas& canvas) override;
    bool handleInput(const InputEvent& event) override;

private:
    void reset();
    void toggle();

    Theme theme_ = defaultTheme();
    bool running_ = false;
    int remaining_seconds_ = 25 * 60;
    uint32_t accumulator_ms_ = 0;
};

} // namespace arfi
