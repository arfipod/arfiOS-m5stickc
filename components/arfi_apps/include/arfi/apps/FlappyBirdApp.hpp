#pragma once

#include "arfi/core/App.hpp"

#include <array>
#include <cstdint>

namespace arfi {

class FlappyBirdApp final : public App {
public:
    explicit FlappyBirdApp(SystemContext& ctx) : App(ctx) {}

    const char* id() const override { return "flappy_bird"; }
    const char* name() const override { return "Flappy Bird"; }

    void onEnter() override;
    void update(uint32_t dt_ms) override;
    void render(Canvas& canvas) override;
    bool handleInput(const InputEvent& event) override;

private:
    enum class State : uint8_t {
        Ready,
        Running,
        GameOver,
    };

    struct Pipe {
        float x = 0.0f;
        int gap_y = 68;
        bool scored = false;
    };

    void reset();
    void flap();
    void updatePipe(Pipe& pipe, float dt_s);
    bool hitsPipe(const Pipe& pipe) const;
    int nextGapY();
    uint32_t nextRandom();

    State state_ = State::Ready;
    float bird_y_ = 64.0f;
    float velocity_ = 0.0f;
    std::array<Pipe, 3> pipes_;
    uint16_t score_ = 0;
    uint16_t best_score_ = 0;
    uint32_t rng_ = 0xC0FFEEu;
};

} // namespace arfi
