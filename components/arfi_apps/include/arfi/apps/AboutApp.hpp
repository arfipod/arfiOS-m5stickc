#pragma once

#include "arfi/core/App.hpp"
#include "arfi/ui/Theme.hpp"

namespace arfi {

class AboutApp final : public App {
public:
    explicit AboutApp(SystemContext& ctx) : App(ctx) {}

    const char* id() const override { return "about"; }
    const char* name() const override { return "About"; }

    void update(uint32_t dt_ms) override;
    void render(Canvas& canvas) override;
    bool handleInput(const InputEvent& event) override;

private:
    Theme theme_ = defaultTheme();
};

} // namespace arfi
