#pragma once

#include "arfi/core/App.hpp"
#include "arfi/ui/Color.hpp"

#include <array>
#include <cstdint>

namespace arfi {

class Cube3DApp final : public App {
public:
    explicit Cube3DApp(SystemContext& ctx) : App(ctx) {}

    const char* id() const override { return "cube3d"; }
    const char* name() const override { return "3D Cube"; }

    void onEnter() override;
    void update(uint32_t dt_ms) override;
    void render(Canvas& canvas) override;
    bool handleInput(const InputEvent& event) override;

private:
    struct Vec3 {
        float x = 0.0f;
        float y = 0.0f;
        float z = 0.0f;
    };

    struct Point2 {
        int x = 0;
        int y = 0;
    };

    void reset();
    void drawLine(Canvas& canvas, Point2 a, Point2 b, Color color) const;
    void drawProjectedCube(Canvas& canvas, const std::array<Point2, 8>& points) const;
    std::array<Point2, 8> projectCube(int cx, int cy) const;

    float angle_x_ = 0.0f;
    float angle_y_ = 0.0f;
    float angle_z_ = 0.0f;
    float speed_ = 1.0f;
    bool paused_ = false;
};

} // namespace arfi
