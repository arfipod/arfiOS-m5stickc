#include "arfi/core/AppRegistry.hpp"
#include "arfi/core/App.hpp"

namespace arfi {

bool AppRegistry::add(const AppDescriptor& descriptor) {
    if (count_ >= kMaxApps || descriptor.instance == nullptr || descriptor.id == nullptr) {
        return false;
    }

    apps_[count_++] = descriptor;
    return true;
}

const AppDescriptor* AppRegistry::at(size_t index) const {
    if (index >= count_) {
        return nullptr;
    }
    return &apps_[index];
}

App* AppRegistry::findAppById(const char* id) const {
    if (id == nullptr) {
        return nullptr;
    }

    for (size_t i = 0; i < count_; ++i) {
        if (std::strcmp(apps_[i].id, id) == 0) {
            return apps_[i].instance;
        }
    }

    return nullptr;
}

size_t AppRegistry::count() const { return count_; }

} // namespace arfi
