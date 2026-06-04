#pragma once

#include "arfi/core/AppIcon.hpp"

#include <cstddef>
#include <cstring>

namespace arfi {

class App;

struct AppDescriptor {
    const char* id = "";
    const char* name = "";
    const char* category = "";
    const char* glyph = "";
    App* instance = nullptr;
    const AppIcon* icon = nullptr;
};

class AppRegistry {
public:
    bool add(const AppDescriptor& descriptor);
    const AppDescriptor* at(size_t index) const;
    App* findAppById(const char* id) const;
    size_t count() const;

private:
    static constexpr size_t kMaxApps = 32;
    AppDescriptor apps_[kMaxApps] = {};
    size_t count_ = 0;
};

} // namespace arfi
