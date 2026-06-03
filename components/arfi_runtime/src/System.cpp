#include "arfi/runtime/System.hpp"

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
      about_app_(ctx_) {
    ctx_.board = makeM5StickCPlusBoardConfig();
    ctx_.app_manager = &app_manager_;
    ctx_.app_registry = &registry_;
    ctx_.display = &display_;
    ctx_.input = &input_;
    ctx_.settings = &settings_;
    ctx_.power = &power_;
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

    registerApps();

    last_tick_ms_ = nowMs();
    last_render_ms_ = 0;
    app_manager_.launch(&launcher_app_);

    ESP_LOGI(TAG, "arfiOS ready");
    return ESP_OK;
}

void System::registerApps() {
    registry_.add({"settings", "Settings", "System", "ST", &settings_app_});
    registry_.add({"diagnostics", "Diagnostics", "System", "DG", &diagnostics_app_});
    registry_.add({"pomodoro", "Pomodoro", "Tools", "PO", &pomodoro_app_});
    registry_.add({"about", "About", "System", "AR", &about_app_});
}

void System::tick() {
    const uint32_t now = nowMs();
    const uint32_t dt = now - last_tick_ms_;
    last_tick_ms_ = now;

    input_.update(now);
    processInput();

    app_manager_.update(dt);

    if (shouldRender(now)) {
        Canvas& canvas = display_.canvas();
        app_manager_.render(canvas);
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

uint32_t System::nowMs() const {
    return static_cast<uint32_t>(esp_timer_get_time() / 1000ULL);
}

} // namespace arfi
