#pragma once

#include "arfi/core/App.hpp"
#include "arfi/services/ImuService.hpp"
#include "arfi/ui/Theme.hpp"

#include "esp_err.h"

namespace arfi {

class ImuLevelApp final : public App {
public:
    explicit ImuLevelApp(SystemContext& ctx) : App(ctx) {}

    const char* id() const override { return "imu_level"; }
    const char* name() const override { return "Nivel IMU"; }

    void onEnter() override;
    void update(uint32_t dt_ms) override;
    void render(Canvas& canvas) override;
    bool handleInput(const InputEvent& event) override;

private:
    void zero();
    void updateAngles();

    Theme theme_ = defaultTheme();
    ImuSample sample_;
    bool has_sample_ = false;
    bool raw_view_ = false;
    esp_err_t last_error_ = ESP_ERR_INVALID_STATE;
    uint32_t sample_accumulator_ms_ = 0;
    float zero_x_g_ = 0.0f;
    float zero_y_g_ = 0.0f;
    float roll_deg_ = 0.0f;
    float pitch_deg_ = 0.0f;
};

} // namespace arfi
