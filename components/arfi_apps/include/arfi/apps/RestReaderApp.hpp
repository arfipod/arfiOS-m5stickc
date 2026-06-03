#pragma once

#include "arfi/core/App.hpp"
#include "arfi/services/RestService.hpp"
#include "arfi/ui/Theme.hpp"

#include <cstdint>
#include <string>

namespace arfi {

class RestReaderApp final : public App {
public:
    explicit RestReaderApp(SystemContext& ctx) : App(ctx) {}

    const char* id() const override { return "rest_reader"; }
    const char* name() const override { return "REST API"; }

    void onEnter() override;
    void update(uint32_t dt_ms) override;
    void render(Canvas& canvas) override;
    bool handleInput(const InputEvent& event) override;

private:
    enum class State : uint8_t {
        Idle,
        Fetching,
        Done,
        Error,
    };

    void fetch();
    void extractReading();
    void drawSanitizedLine(Canvas& canvas, int x, int y, const std::string& text, Color color, size_t offset) const;
    void drawBodyPreview(Canvas& canvas, int x, int y, Color color) const;
    static const char* stateName_(State state);
    static char sanitize_(char c);

    Theme theme_ = defaultTheme();
    State state_ = State::Idle;
    RestResponse response_;
    std::string endpoint_;
    std::string reading_;
    bool raw_view_ = false;
    uint32_t request_count_ = 0;
};

} // namespace arfi
