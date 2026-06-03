#include "arfi/apps/IrSweepApp.hpp"
#include "arfi/core/SystemContext.hpp"
#include "arfi/services/IrService.hpp"
#include "arfi/ui/Canvas.hpp"

#include <cstdio>

namespace arfi {

namespace {
constexpr uint32_t kBurstIntervalMs = 120;
constexpr uint16_t kBurstDurationMs = 46;
} // namespace

void IrSweepApp::onEnter() {
    running_ = false;
    accumulator_ms_ = kBurstIntervalMs;
    bursts_ = 0;
    last_error_ = ESP_ERR_INVALID_STATE;
    resetCarrier();
}

void IrSweepApp::onExit() {
    if (ctx_.ir != nullptr) {
        ctx_.ir->stop();
    }
}

void IrSweepApp::update(uint32_t dt_ms) {
    if (!running_) {
        return;
    }

    accumulator_ms_ += dt_ms;
    if (accumulator_ms_ < kBurstIntervalMs) {
        return;
    }
    accumulator_ms_ = 0;
    emitStep();
}

void IrSweepApp::render(Canvas& canvas) {
    canvas.clear(theme_.background);
    canvas.drawText(6, 4, "BARRIDO IR", theme_.text, 1);
    canvas.hline(0, 16, canvas.width(), theme_.surface_alt);

    const bool available = ctx_.ir != nullptr && ctx_.ir->available();
    if (!available) {
        canvas.drawCenteredText(canvas.width() / 2, 48, "IR NOT READY", theme_.warning, 2);
        canvas.drawText(8, 104, "GPIO9 IR LED", theme_.muted, 1);
        canvas.drawText(8, 122, "A LONG HOME", theme_.muted, 1);
        return;
    }

    char line[48];
    std::snprintf(line, sizeof(line), "%s %s", running_ ? "RUN" : "STOP", modeName());
    canvas.drawCenteredText(canvas.width() / 2, 30, line, running_ ? theme_.accent : theme_.muted, 1);

    std::snprintf(line, sizeof(line), "%lu KHZ", static_cast<unsigned long>(carrier_hz_ / 1000));
    canvas.drawCenteredText(canvas.width() / 2, 52, line, theme_.text, 3);

    const uint32_t min_hz = minCarrierHz();
    const uint32_t max_hz = maxCarrierHz();
    const int bar_x = 22;
    const int bar_y = 92;
    const int bar_w = canvas.width() - 44;
    const int bar_h = 10;
    int marker_x = bar_x + bar_w / 2;
    if (max_hz > min_hz) {
        marker_x = bar_x + static_cast<int>(((carrier_hz_ - min_hz) * static_cast<uint32_t>(bar_w - 1)) / (max_hz - min_hz));
    }
    canvas.drawRect(bar_x, bar_y, bar_w, bar_h, theme_.surface_alt);
    canvas.fillRect(bar_x + 1, bar_y + 1, marker_x - bar_x, bar_h - 2, theme_.accent);
    canvas.vline(marker_x, bar_y - 4, bar_h + 8, theme_.warning);

    std::snprintf(
        line,
        sizeof(line),
        "%lu-%luK  N%lu",
        static_cast<unsigned long>(min_hz / 1000),
        static_cast<unsigned long>(max_hz / 1000),
        static_cast<unsigned long>(bursts_));
    canvas.drawCenteredText(canvas.width() / 2, 76, line, theme_.muted, 1);

    canvas.drawText(8, 112, "A RUN/STOP  B MODE", theme_.muted, 1);
    canvas.drawText(8, 126, "A LONG HOME", theme_.muted, 1);
}

bool IrSweepApp::handleInput(const InputEvent& event) {
    if (event.key == Key::Primary && event.type == InputEventType::ShortPress) {
        toggle();
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::ShortPress) {
        nextMode();
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::LongPress) {
        resetCarrier();
        return true;
    }

    return false;
}

void IrSweepApp::toggle() {
    running_ = !running_;
    accumulator_ms_ = kBurstIntervalMs;
    if (!running_ && ctx_.ir != nullptr) {
        ctx_.ir->stop();
    }
}

void IrSweepApp::nextMode() {
    if (mode_ == SweepMode::Remote) {
        mode_ = SweepMode::Wide;
    } else if (mode_ == SweepMode::Wide) {
        mode_ = SweepMode::Fixed;
    } else {
        mode_ = SweepMode::Remote;
    }
    resetCarrier();
}

void IrSweepApp::resetCarrier() {
    carrier_hz_ = minCarrierHz();
    if (mode_ == SweepMode::Fixed) {
        carrier_hz_ = 38000;
    }
    direction_ = 1;
}

void IrSweepApp::emitStep() {
    if (ctx_.ir == nullptr || !ctx_.ir->available()) {
        running_ = false;
        return;
    }

    last_error_ = ctx_.ir->emitBurst(carrier_hz_, kBurstDurationMs);
    if (last_error_ != ESP_OK) {
        running_ = false;
        return;
    }

    ++bursts_;
    if (mode_ == SweepMode::Fixed) {
        return;
    }

    const uint32_t min_hz = minCarrierHz();
    const uint32_t max_hz = maxCarrierHz();
    const uint32_t step_hz = carrierStepHz();

    if (direction_ > 0 && carrier_hz_ + step_hz >= max_hz) {
        carrier_hz_ = max_hz;
        direction_ = -1;
    } else if (direction_ < 0 && carrier_hz_ <= min_hz + step_hz) {
        carrier_hz_ = min_hz;
        direction_ = 1;
    } else if (direction_ > 0) {
        carrier_hz_ += step_hz;
    } else {
        carrier_hz_ -= step_hz;
    }
}

uint32_t IrSweepApp::minCarrierHz() const {
    if (mode_ == SweepMode::Wide) {
        return 30000;
    }
    if (mode_ == SweepMode::Fixed) {
        return 38000;
    }
    return 36000;
}

uint32_t IrSweepApp::maxCarrierHz() const {
    if (mode_ == SweepMode::Wide) {
        return 60000;
    }
    if (mode_ == SweepMode::Fixed) {
        return 38000;
    }
    return 40000;
}

uint32_t IrSweepApp::carrierStepHz() const {
    if (mode_ == SweepMode::Wide) {
        return 2000;
    }
    return 1000;
}

const char* IrSweepApp::modeName() const {
    if (mode_ == SweepMode::Wide) {
        return "30-60K";
    }
    if (mode_ == SweepMode::Fixed) {
        return "38K";
    }
    return "36-40K";
}

} // namespace arfi
