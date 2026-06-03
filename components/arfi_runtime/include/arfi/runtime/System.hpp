#pragma once

#include "arfi/core/AppManager.hpp"
#include "arfi/core/AppRegistry.hpp"
#include "arfi/core/SystemContext.hpp"
#include "arfi/services/DisplayService.hpp"
#include "arfi/services/ImuService.hpp"
#include "arfi/services/InputService.hpp"
#include "arfi/services/IrService.hpp"
#include "arfi/services/PowerService.hpp"
#include "arfi/services/RestService.hpp"
#include "arfi/services/SettingsService.hpp"
#include "arfi/services/WifiService.hpp"
#include "arfi/apps/AboutApp.hpp"
#include "arfi/apps/DiagnosticsApp.hpp"
#include "arfi/apps/FlappyBirdApp.hpp"
#include "arfi/apps/ImuLevelApp.hpp"
#include "arfi/apps/IrSweepApp.hpp"
#include "arfi/apps/LauncherApp.hpp"
#include "arfi/apps/PomodoroApp.hpp"
#include "arfi/apps/RestReaderApp.hpp"
#include "arfi/apps/SettingsApp.hpp"

#include "esp_err.h"

namespace arfi {

class System {
public:
    System();

    esp_err_t begin();
    void tick();

private:
    void registerApps();
    void processInput();
    bool shouldRender(uint32_t now_ms) const;
    uint32_t nowMs() const;

    SystemContext ctx_;
    SettingsService settings_;
    PowerService power_;
    DisplayService display_;
    InputService input_;
    ImuService imu_;
    IrService ir_;
    WifiService wifi_;
    RestService rest_;
    AppRegistry registry_;
    AppManager app_manager_;

    LauncherApp launcher_app_;
    SettingsApp settings_app_;
    DiagnosticsApp diagnostics_app_;
    PomodoroApp pomodoro_app_;
    ImuLevelApp imu_level_app_;
    IrSweepApp ir_sweep_app_;
    FlappyBirdApp flappy_bird_app_;
    RestReaderApp rest_reader_app_;
    AboutApp about_app_;

    uint32_t last_tick_ms_ = 0;
    uint32_t last_render_ms_ = 0;
};

} // namespace arfi
