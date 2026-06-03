#pragma once

#include <cstdint>

#include "arfi/core/InputEvent.hpp"

namespace arfi {

class Canvas;
struct SystemContext;

class App {
public:
    explicit App(SystemContext& ctx) : ctx_(ctx) {}
    virtual ~App() = default;

    virtual const char* id() const = 0;
    virtual const char* name() const = 0;

    virtual void onEnter() {}
    virtual void onExit() {}

    virtual void update(uint32_t dt_ms) = 0;
    virtual void render(Canvas& canvas) = 0;
    virtual bool handleInput(const InputEvent& event) = 0;

protected:
    SystemContext& ctx_;
};

} // namespace arfi
