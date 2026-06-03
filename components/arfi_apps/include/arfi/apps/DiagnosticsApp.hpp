#pragma once

#include "arfi/core/App.hpp"
#include "arfi/ui/Theme.hpp"

namespace arfi {

class DiagnosticsApp final : public App {
public:
    explicit DiagnosticsApp(SystemContext& ctx) : App(ctx) {}

    const char* id() const override { return "diagnostics"; }
    const char* name() const override { return "Diagnostics"; }

    void update(uint32_t dt_ms) override;
    void render(Canvas& canvas) override;
    bool handleInput(const InputEvent& event) override;

private:
    Theme theme_ = defaultTheme();
    uint32_t uptime_ms_ = 0;
    const char* last_event_ = "NONE";
};

} // namespace arfi
