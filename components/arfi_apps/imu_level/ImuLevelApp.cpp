#include "arfi/apps/ImuLevelApp.hpp"
#include "arfi/core/SystemContext.hpp"
#include "arfi/services/ImuService.hpp"
#include "arfi/ui/Canvas.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstdlib>

namespace arfi {

namespace {
constexpr uint32_t kSamplePeriodMs = 40;
constexpr float kRadToDeg = 57.2957795f;

int roundedTenths(float value) {
    return static_cast<int>((value * 10.0f) + (value >= 0.0f ? 0.5f : -0.5f));
}

void formatFixed(char* line, size_t length, const char* label, float value, const char* unit) {
    const int tenths = roundedTenths(value);
    const int abs_tenths = std::abs(tenths);
    std::snprintf(
        line,
        length,
        "%s %c%d.%d%s",
        label,
        tenths < 0 ? '-' : '+',
        abs_tenths / 10,
        abs_tenths % 10,
        unit);
}
} // namespace

void ImuLevelApp::onEnter() {
    sample_accumulator_ms_ = kSamplePeriodMs;
    has_sample_ = false;
    last_error_ = ESP_ERR_INVALID_STATE;
}

void ImuLevelApp::update(uint32_t dt_ms) {
    sample_accumulator_ms_ += dt_ms;
    if (sample_accumulator_ms_ < kSamplePeriodMs) {
        return;
    }
    sample_accumulator_ms_ = 0;

    if (ctx_.imu == nullptr || !ctx_.imu->available()) {
        has_sample_ = false;
        last_error_ = ctx_.imu == nullptr ? ESP_ERR_INVALID_STATE : ctx_.imu->lastError();
        return;
    }

    last_error_ = ctx_.imu->read(sample_);
    has_sample_ = last_error_ == ESP_OK;
    if (has_sample_) {
        updateAngles();
    }
}

void ImuLevelApp::render(Canvas& canvas) {
    canvas.clear(theme_.background);
    canvas.drawText(6, 4, "NIVEL IMU", theme_.text, 1);
    canvas.hline(0, 16, canvas.width(), theme_.surface_alt);

    if (!has_sample_) {
        canvas.drawCenteredText(canvas.width() / 2, 48, "IMU NOT READY", theme_.warning, 2);
        char line[32];
        std::snprintf(line, sizeof(line), "ERR 0X%X", static_cast<unsigned>(last_error_));
        canvas.drawCenteredText(canvas.width() / 2, 78, line, theme_.muted, 1);
        canvas.drawText(8, 118, "A LONG HOME", theme_.muted, 1);
        return;
    }

    if (raw_view_) {
        char line[36];
        formatFixed(line, sizeof(line), "AX", sample_.accel_x_g, "G");
        canvas.drawText(10, 30, line, theme_.text, 1);
        formatFixed(line, sizeof(line), "AY", sample_.accel_y_g, "G");
        canvas.drawText(10, 46, line, theme_.text, 1);
        formatFixed(line, sizeof(line), "AZ", sample_.accel_z_g, "G");
        canvas.drawText(10, 62, line, theme_.text, 1);
        formatFixed(line, sizeof(line), "GX", sample_.gyro_x_dps, "D");
        canvas.drawText(122, 30, line, theme_.muted, 1);
        formatFixed(line, sizeof(line), "GY", sample_.gyro_y_dps, "D");
        canvas.drawText(122, 46, line, theme_.muted, 1);
        formatFixed(line, sizeof(line), "GZ", sample_.gyro_z_dps, "D");
        canvas.drawText(122, 62, line, theme_.muted, 1);
    } else {
        const int box_x = 18;
        const int box_y = 28;
        const int box_w = 132;
        const int box_h = 76;
        const int center_x = box_x + box_w / 2;
        const int center_y = box_y + box_h / 2;
        const int offset_x = static_cast<int>(std::clamp((sample_.accel_y_g - zero_y_g_) * 54.0f, -54.0f, 54.0f));
        const int offset_y = static_cast<int>(std::clamp((sample_.accel_x_g - zero_x_g_) * -32.0f, -32.0f, 32.0f));

        canvas.drawRect(box_x, box_y, box_w, box_h, theme_.surface_alt);
        canvas.hline(box_x + 4, center_y, box_w - 8, theme_.surface_alt);
        canvas.vline(center_x, box_y + 4, box_h - 8, theme_.surface_alt);
        canvas.drawRect(center_x - 13, center_y - 13, 27, 27, theme_.muted);

        const int bubble_x = center_x + offset_x - 7;
        const int bubble_y = center_y + offset_y - 7;
        canvas.fillRect(bubble_x, bubble_y, 15, 15, theme_.accent);
        canvas.drawRect(bubble_x - 1, bubble_y - 1, 17, 17, Colors::Black);

        char line[36];
        formatFixed(line, sizeof(line), "ROLL", roll_deg_, "");
        canvas.drawText(162, 34, line, theme_.text, 1);
        formatFixed(line, sizeof(line), "PITCH", pitch_deg_, "");
        canvas.drawText(162, 52, line, theme_.text, 1);
        formatFixed(line, sizeof(line), "TEMP", sample_.temperature_c, "C");
        canvas.drawText(162, 70, line, theme_.muted, 1);
    }

    canvas.drawText(8, 112, "A ZERO  B RAW", theme_.muted, 1);
    canvas.drawText(8, 126, "A LONG HOME", theme_.muted, 1);
}

bool ImuLevelApp::handleInput(const InputEvent& event) {
    if (event.key == Key::Primary && event.type == InputEventType::ShortPress) {
        zero();
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::ShortPress) {
        raw_view_ = !raw_view_;
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::LongPress) {
        zero_x_g_ = 0.0f;
        zero_y_g_ = 0.0f;
        return true;
    }

    return false;
}

void ImuLevelApp::zero() {
    if (!has_sample_) {
        return;
    }

    zero_x_g_ = sample_.accel_x_g;
    zero_y_g_ = sample_.accel_y_g;
}

void ImuLevelApp::updateAngles() {
    roll_deg_ = std::atan2(sample_.accel_y_g, sample_.accel_z_g) * kRadToDeg;
    const float yz = std::sqrt((sample_.accel_y_g * sample_.accel_y_g) + (sample_.accel_z_g * sample_.accel_z_g));
    pitch_deg_ = std::atan2(-sample_.accel_x_g, yz) * kRadToDeg;
}

} // namespace arfi
