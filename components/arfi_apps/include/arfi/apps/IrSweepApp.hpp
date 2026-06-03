#pragma once

#include "arfi/core/App.hpp"
#include "arfi/ui/Theme.hpp"

#include "esp_err.h"

#include <cstdint>

namespace arfi {

class IrSweepApp final : public App {
public:
    explicit IrSweepApp(SystemContext& ctx) : App(ctx) {}

    const char* id() const override { return "ir_sweep"; }
    const char* name() const override { return "Barrido IR"; }

    void onEnter() override;
    void onExit() override;
    void update(uint32_t dt_ms) override;
    void render(Canvas& canvas) override;
    bool handleInput(const InputEvent& event) override;

private:
    enum class SweepMode : uint8_t {
        Remote,
        Wide,
        Fixed,
    };

    void toggle();
    void nextMode();
    void resetCarrier();
    void emitStep();
    uint32_t minCarrierHz() const;
    uint32_t maxCarrierHz() const;
    uint32_t carrierStepHz() const;
    const char* modeName() const;

    Theme theme_ = defaultTheme();
    SweepMode mode_ = SweepMode::Remote;
    bool running_ = false;
    uint32_t accumulator_ms_ = 0;
    uint32_t carrier_hz_ = 36000;
    int8_t direction_ = 1;
    uint32_t bursts_ = 0;
    esp_err_t last_error_ = ESP_ERR_INVALID_STATE;
};

} // namespace arfi
