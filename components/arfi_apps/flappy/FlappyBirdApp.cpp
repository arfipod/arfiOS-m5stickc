#include "arfi/apps/FlappyBirdApp.hpp"
#include "arfi/ui/Canvas.hpp"

#include <algorithm>
#include <cstdio>

namespace arfi {

namespace {
constexpr int kScreenW = 240;
constexpr int kScreenH = 135;
constexpr int kGroundY = 122;
constexpr int kBirdX = 46;
constexpr int kBirdSize = 9;
constexpr int kPipeW = 19;
constexpr int kGapH = 46;
constexpr int kPipeSpacing = 82;
constexpr float kGravity = 430.0f;
constexpr float kFlapVelocity = -165.0f;
constexpr float kPipeSpeed = 76.0f;

constexpr Color kSky = Color::rgb(66, 178, 214);
constexpr Color kGround = Color::rgb(126, 200, 78);
constexpr Color kDirt = Color::rgb(104, 74, 52);
constexpr Color kPipe = Color::rgb(28, 156, 78);
constexpr Color kPipeDark = Color::rgb(14, 86, 46);
constexpr Color kBird = Color::rgb(255, 216, 54);
constexpr Color kBeak = Color::rgb(244, 92, 48);
} // namespace

void FlappyBirdApp::onEnter() { reset(); }

void FlappyBirdApp::update(uint32_t dt_ms) {
    if (state_ != State::Running) {
        return;
    }

    const float dt_s = static_cast<float>(std::min<uint32_t>(dt_ms, 50)) / 1000.0f;
    velocity_ += kGravity * dt_s;
    bird_y_ += velocity_ * dt_s;

    for (auto& pipe : pipes_) {
        updatePipe(pipe, dt_s);
        if (!pipe.scored && pipe.x + kPipeW < kBirdX) {
            pipe.scored = true;
            ++score_;
            best_score_ = std::max(best_score_, score_);
        }
        if (hitsPipe(pipe)) {
            state_ = State::GameOver;
        }
    }

    if (bird_y_ < 16.0f || bird_y_ + kBirdSize >= kGroundY) {
        state_ = State::GameOver;
    }
}

void FlappyBirdApp::render(Canvas& canvas) {
    canvas.clear(kSky);

    canvas.fillRect(24, 24, 18, 5, Colors::White);
    canvas.fillRect(34, 20, 20, 7, Colors::White);
    canvas.fillRect(168, 34, 24, 6, Colors::White);
    canvas.fillRect(184, 30, 16, 8, Colors::White);

    for (const auto& pipe : pipes_) {
        const int x = static_cast<int>(pipe.x);
        const int gap_top = pipe.gap_y - kGapH / 2;
        const int gap_bottom = pipe.gap_y + kGapH / 2;
        canvas.fillRect(x, 0, kPipeW, gap_top, kPipe);
        canvas.fillRect(x - 2, gap_top - 7, kPipeW + 4, 7, kPipeDark);
        canvas.fillRect(x, gap_bottom, kPipeW, kGroundY - gap_bottom, kPipe);
        canvas.fillRect(x - 2, gap_bottom, kPipeW + 4, 7, kPipeDark);
    }

    const int bird_y = static_cast<int>(bird_y_);
    canvas.fillRect(kBirdX, bird_y, kBirdSize, kBirdSize, kBird);
    canvas.fillRect(kBirdX + 6, bird_y + 3, 5, 3, kBeak);
    canvas.fillRect(kBirdX + 2, bird_y + 2, 2, 2, Colors::Black);
    canvas.fillRect(kBirdX - 2, bird_y + 4, 4, 3, Color::rgb(255, 237, 130));

    canvas.fillRect(0, kGroundY, kScreenW, kScreenH - kGroundY, kGround);
    canvas.hline(0, kGroundY, kScreenW, kDirt);

    char line[32];
    std::snprintf(line, sizeof(line), "%u", static_cast<unsigned>(score_));
    canvas.drawCenteredText(kScreenW / 2, 6, line, Colors::White, 2);
    std::snprintf(line, sizeof(line), "BEST %u", static_cast<unsigned>(best_score_));
    canvas.drawText(6, 6, line, Colors::White, 1);

    if (state_ == State::Ready) {
        canvas.drawCenteredText(kScreenW / 2, 46, "A FLAP", Colors::White, 2);
        canvas.drawCenteredText(kScreenW / 2, 72, "B RESET", Colors::White, 1);
    } else if (state_ == State::GameOver) {
        canvas.drawCenteredText(kScreenW / 2, 44, "GAME OVER", Colors::White, 2);
        canvas.drawCenteredText(kScreenW / 2, 70, "A RETRY", Colors::White, 1);
    }
}

bool FlappyBirdApp::handleInput(const InputEvent& event) {
    if (event.key == Key::Primary && event.type == InputEventType::Pressed) {
        flap();
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::ShortPress) {
        reset();
        return true;
    }

    return false;
}

void FlappyBirdApp::reset() {
    state_ = State::Ready;
    bird_y_ = 60.0f;
    velocity_ = 0.0f;
    score_ = 0;

    for (size_t i = 0; i < pipes_.size(); ++i) {
        pipes_[i].x = 170.0f + static_cast<float>(i * kPipeSpacing);
        pipes_[i].gap_y = nextGapY();
        pipes_[i].scored = false;
    }
}

void FlappyBirdApp::flap() {
    if (state_ == State::GameOver) {
        reset();
    }
    state_ = State::Running;
    velocity_ = kFlapVelocity;
}

void FlappyBirdApp::updatePipe(Pipe& pipe, float dt_s) {
    pipe.x -= kPipeSpeed * dt_s;
    if (pipe.x + kPipeW >= 0.0f) {
        return;
    }

    float rightmost = pipe.x;
    for (const auto& other : pipes_) {
        rightmost = std::max(rightmost, other.x);
    }

    pipe.x = rightmost + kPipeSpacing;
    pipe.gap_y = nextGapY();
    pipe.scored = false;
}

bool FlappyBirdApp::hitsPipe(const Pipe& pipe) const {
    const float bird_left = static_cast<float>(kBirdX);
    const float bird_right = static_cast<float>(kBirdX + kBirdSize);
    const float bird_top = bird_y_;
    const float bird_bottom = bird_y_ + kBirdSize;

    if (bird_right < pipe.x || bird_left > pipe.x + kPipeW) {
        return false;
    }

    const float gap_top = static_cast<float>(pipe.gap_y - kGapH / 2);
    const float gap_bottom = static_cast<float>(pipe.gap_y + kGapH / 2);
    return bird_top < gap_top || bird_bottom > gap_bottom;
}

int FlappyBirdApp::nextGapY() {
    return 38 + static_cast<int>(nextRandom() % 49);
}

uint32_t FlappyBirdApp::nextRandom() {
    rng_ = (rng_ * 1664525u) + 1013904223u;
    return rng_;
}

} // namespace arfi
