#include "arfi/runtime/System.hpp"

#include "arfi/apps/AppIcons.hpp"

#include "esp_check.h"
#include "esp_log.h"
#include "esp_timer.h"
#include "nvs_flash.h"

namespace arfi {

static const char* TAG = "arfi_system";

System::System()
    : launcher_app_(ctx_),
      settings_app_(ctx_),
      diagnostics_app_(ctx_),
      pomodoro_app_(ctx_),
      imu_level_app_(ctx_),
      ir_sweep_app_(ctx_),
      flappy_bird_app_(ctx_),
      rest_reader_app_(ctx_),
      about_app_(ctx_) {
    ctx_.board = makeM5StickCPlusBoardConfig();
    ctx_.app_manager = &app_manager_;
    ctx_.app_registry = &registry_;
    ctx_.display = &display_;
    ctx_.imu = &imu_;
    ctx_.input = &input_;
    ctx_.ir = &ir_;
    ctx_.rest = &rest_;
    ctx_.settings = &settings_;
    ctx_.power = &power_;
    ctx_.wifi = &wifi_;
}

esp_err_t System::begin() {
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        err = nvs_flash_init();
    }
    ESP_RETURN_ON_ERROR(err, TAG, "NVS initialization failed");

    ESP_RETURN_ON_ERROR(settings_.begin(), TAG, "Settings initialization failed");
    ESP_RETURN_ON_ERROR(power_.begin(ctx_.board), TAG, "Power initialization failed");
    ESP_RETURN_ON_ERROR(display_.begin(ctx_.board, power_), TAG, "Display initialization failed");
    power_.setBacklightPercent(static_cast<uint8_t>(settings_.getInt("ui_bright", 90)));
    ESP_RETURN_ON_ERROR(input_.begin(ctx_.board), TAG, "Input initialization failed");

    esp_err_t optional_err = imu_.begin(ctx_.board);
    if (optional_err != ESP_OK) {
        ESP_LOGW(TAG, "IMU service unavailable: %s", esp_err_to_name(optional_err));
    }

    optional_err = ir_.begin(ctx_.board);
    if (optional_err != ESP_OK) {
        ESP_LOGW(TAG, "IR service unavailable: %s", esp_err_to_name(optional_err));
    }

    optional_err = wifi_.begin();
    if (optional_err != ESP_OK) {
        ESP_LOGW(TAG, "Wi-Fi service unavailable: %s", esp_err_to_name(optional_err));
    }

    optional_err = rest_.begin(wifi_);
    if (optional_err != ESP_OK) {
        ESP_LOGW(TAG, "REST service unavailable: %s", esp_err_to_name(optional_err));
    }

    registerApps();

    last_tick_ms_ = nowMs();
    last_render_ms_ = 0;
    app_manager_.launch(&launcher_app_);

    ESP_LOGI(TAG, "arfiOS ready");
    return ESP_OK;
}

void System::registerApps() {
    registry_.add({"settings", "Settings", "System", "ST", &settings_app_, &app_icons::Settings});
    registry_.add({"diagnostics", "Diagnostics", "System", "DG", &diagnostics_app_, &app_icons::Diagnostics});
    registry_.add({"pomodoro", "Pomodoro", "Tools", "PO", &pomodoro_app_, &app_icons::Pomodoro});
    registry_.add({"imu_level", "Nivel IMU", "Tools", "LV", &imu_level_app_, &app_icons::ImuLevel});
    registry_.add({"ir_sweep", "Barrido IR", "Tools", "IR", &ir_sweep_app_, &app_icons::IrSweep});
    registry_.add({"flappy_bird", "Flappy Bird", "Games", "FB", &flappy_bird_app_, &app_icons::FlappyBird});
    registry_.add({"rest_reader", "REST API", "Tools", "RS", &rest_reader_app_, &app_icons::Rest});
    registry_.add({"about", "About", "System", "AR", &about_app_, &app_icons::About});
}

void System::tick() {
    const uint32_t now = nowMs();
    const uint32_t dt = now - last_tick_ms_;
    last_tick_ms_ = now;

    input_.update(now);
    ir_.update(now);
    processInput();

    app_manager_.update(dt);

    if (shouldRender(now)) {
        Canvas& canvas = display_.canvas();
        app_manager_.render(canvas);
        renderStatusOverlay(canvas);
        display_.flush();
        last_render_ms_ = now;
    }
}

void System::processInput() {
    InputEvent event;
    while (input_.poll(event)) {
        const bool on_launcher = app_manager_.current() == &launcher_app_;
        if (!on_launcher && event.key == Key::Primary && event.type == InputEventType::LongPress) {
            app_manager_.launch(&launcher_app_);
            continue;
        }

        app_manager_.handleInput(event);
    }
}

bool System::shouldRender(uint32_t now_ms) const {
    return now_ms - last_render_ms_ >= 33;
}

void System::renderStatusOverlay(Canvas& canvas) {
    if (!wifi_.connected()) {
        return;
    }

    const uint8_t level = wifi_.signalLevel();
    const int base_x = static_cast<int>(canvas.width()) - 18;
    const int base_y = 13;
    const int bar_w = 2;
    const int gap = 2;
    const Color active = level >= 3 ? Colors::Green : Colors::Yellow;

    canvas.fillRect(base_x - 2, 2, 18, 13, Colors::Black);
    for (uint8_t i = 0; i < 4; ++i) {
        const int h = 3 + static_cast<int>(i) * 2;
        const int x = base_x + static_cast<int>(i) * (bar_w + gap);
        const int y = base_y - h;
        canvas.drawRect(x, y, bar_w, h, Colors::Gray50);
        if (i < level) {
            canvas.fillRect(x, y, bar_w, h, active);
        }
    }
}

uint32_t System::nowMs() const {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
}

} // namespace arfi
