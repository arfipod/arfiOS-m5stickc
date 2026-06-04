#include "arfi/apps/Cube3DApp.hpp"
#include "arfi/ui/Canvas.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>

namespace arfi {

namespace {
constexpr int kScreenW = 240;
constexpr int kScreenH = 135;
constexpr float kCubeSize = 42.0f;
constexpr float kCameraDistance = 3.4f;
constexpr float kProjectionScale = 78.0f;

constexpr Color kBackground = Color::rgb(8, 13, 22);
constexpr Color kGrid = Color::rgb(22, 38, 54);
constexpr Color kBackEdge = Color::rgb(57, 107, 130);
constexpr Color kFrontEdge = Color::rgb(101, 224, 214);
constexpr Color kCorner = Color::rgb(255, 208, 76);
constexpr Color kText = Color::rgb(216, 234, 238);
constexpr Color kMuted = Color::rgb(112, 132, 146);

struct CubeVertex {
    float x = 0.0f;
    float y = 0.0f;
    float z = 0.0f;
};

constexpr std::array<CubeVertex, 8> kVertices = {{
    {-1.0f, -1.0f, -1.0f},
    {1.0f, -1.0f, -1.0f},
    {1.0f, 1.0f, -1.0f},
    {-1.0f, 1.0f, -1.0f},
    {-1.0f, -1.0f, 1.0f},
    {1.0f, -1.0f, 1.0f},
    {1.0f, 1.0f, 1.0f},
    {-1.0f, 1.0f, 1.0f},
}};

constexpr std::array<std::array<uint8_t, 2>, 12> kEdges = {{
    {{0, 1}},
    {{1, 2}},
    {{2, 3}},
    {{3, 0}},
    {{4, 5}},
    {{5, 6}},
    {{6, 7}},
    {{7, 4}},
    {{0, 4}},
    {{1, 5}},
    {{2, 6}},
    {{3, 7}},
}};
} // namespace

void Cube3DApp::onEnter() { reset(); }

void Cube3DApp::update(uint32_t dt_ms) {
    if (paused_) {
        return;
    }

    const float dt_s = static_cast<float>(std::min<uint32_t>(dt_ms, 50)) / 1000.0f;
    angle_x_ += dt_s * 0.82f * speed_;
    angle_y_ += dt_s * 1.08f * speed_;
    angle_z_ += dt_s * 0.37f * speed_;
}

void Cube3DApp::render(Canvas& canvas) {
    canvas.clear(kBackground);

    for (int y = 24; y < kScreenH; y += 18) {
        canvas.hline(0, y, kScreenW, kGrid);
    }
    for (int x = 0; x < kScreenW; x += 24) {
        canvas.vline(x, 18, kScreenH - 18, kGrid);
    }

    canvas.drawText(6, 4, "3D CUBE", kText, 1);
    canvas.hline(0, 16, kScreenW, kGrid);

    const auto points = projectCube(kScreenW / 2, 69);
    drawProjectedCube(canvas, points);

    char line[40];
    std::snprintf(line, sizeof(line), "%s  SPD %.1f", paused_ ? "PAUSE" : "RUN", static_cast<double>(speed_));
    canvas.drawText(8, 112, line, kMuted, 1);
    canvas.drawText(8, 126, "A PAUSE  B SPEED", kMuted, 1);
}

bool Cube3DApp::handleInput(const InputEvent& event) {
    if (event.key == Key::Primary && event.type == InputEventType::ShortPress) {
        paused_ = !paused_;
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::ShortPress) {
        speed_ += 0.5f;
        if (speed_ > 2.0f) {
            speed_ = 0.5f;
        }
        return true;
    }

    if (event.key == Key::Secondary && event.type == InputEventType::LongPress) {
        reset();
        return true;
    }

    return false;
}

void Cube3DApp::reset() {
    angle_x_ = 0.35f;
    angle_y_ = 0.65f;
    angle_z_ = 0.12f;
    speed_ = 1.0f;
    paused_ = false;
}

std::array<Cube3DApp::Point2, 8> Cube3DApp::projectCube(int cx, int cy) const {
    const float sx = std::sin(angle_x_);
    const float cxr = std::cos(angle_x_);
    const float sy = std::sin(angle_y_);
    const float cyr = std::cos(angle_y_);
    const float sz = std::sin(angle_z_);
    const float cz = std::cos(angle_z_);

    std::array<Point2, 8> out = {};
    for (size_t i = 0; i < kVertices.size(); ++i) {
        Vec3 p = {
            kVertices[i].x * kCubeSize,
            kVertices[i].y * kCubeSize,
            kVertices[i].z * kCubeSize,
        };

        const float y1 = p.y * cxr - p.z * sx;
        const float z1 = p.y * sx + p.z * cxr;
        p.y = y1;
        p.z = z1;

        const float x2 = p.x * cyr + p.z * sy;
        const float z2 = -p.x * sy + p.z * cyr;
        p.x = x2;
        p.z = z2;

        const float x3 = p.x * cz - p.y * sz;
        const float y3 = p.x * sz + p.y * cz;
        p.x = x3;
        p.y = y3;

        const float depth = kCameraDistance + (p.z / kCubeSize);
        const float perspective = kProjectionScale / depth;
        out[i].x = cx + static_cast<int>(p.x * perspective / kCubeSize);
        out[i].y = cy + static_cast<int>(p.y * perspective / kCubeSize);
    }
    return out;
}

void Cube3DApp::drawProjectedCube(Canvas& canvas, const std::array<Point2, 8>& points) const {
    for (size_t i = 0; i < kEdges.size(); ++i) {
        const auto edge = kEdges[i];
        const Color color = i < 4 ? kBackEdge : kFrontEdge;
        drawLine(canvas, points[edge[0]], points[edge[1]], color);
    }

    for (const auto& point : points) {
        canvas.fillRect(point.x - 2, point.y - 2, 5, 5, kCorner);
    }
}

void Cube3DApp::drawLine(Canvas& canvas, Point2 a, Point2 b, Color color) const {
    int x0 = a.x;
    int y0 = a.y;
    const int x1 = b.x;
    const int y1 = b.y;
    const int dx = std::abs(x1 - x0);
    const int sx = x0 < x1 ? 1 : -1;
    const int dy = -std::abs(y1 - y0);
    const int sy = y0 < y1 ? 1 : -1;
    int err = dx + dy;

    while (true) {
        canvas.pixel(x0, y0, color);
        if (x0 == x1 && y0 == y1) {
            break;
        }

        const int e2 = err * 2;
        if (e2 >= dy) {
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx) {
            err += dx;
            y0 += sy;
        }
    }
}

} // namespace arfi
