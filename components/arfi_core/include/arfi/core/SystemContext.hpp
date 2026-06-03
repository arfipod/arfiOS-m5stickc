#pragma once

#include "arfi/hal/BoardConfig.hpp"

namespace arfi {

class AppManager;
class AppRegistry;
class DisplayService;
class ImuService;
class InputService;
class IrService;
class RestService;
class SettingsService;
class PowerService;
class WifiService;

struct SystemContext {
    BoardConfig board;
    AppManager* app_manager = nullptr;
    AppRegistry* app_registry = nullptr;
    DisplayService* display = nullptr;
    ImuService* imu = nullptr;
    InputService* input = nullptr;
    IrService* ir = nullptr;
    RestService* rest = nullptr;
    SettingsService* settings = nullptr;
    PowerService* power = nullptr;
    WifiService* wifi = nullptr;
};

} // namespace arfi
