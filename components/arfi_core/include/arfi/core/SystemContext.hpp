#pragma once

#include "arfi/hal/BoardConfig.hpp"

namespace arfi {

class AppManager;
class AppRegistry;
class DisplayService;
class InputService;
class SettingsService;
class PowerService;

struct SystemContext {
    BoardConfig board;
    AppManager* app_manager = nullptr;
    AppRegistry* app_registry = nullptr;
    DisplayService* display = nullptr;
    InputService* input = nullptr;
    SettingsService* settings = nullptr;
    PowerService* power = nullptr;
};

} // namespace arfi
