// Utilities for drawing things in world space.

#pragma once

#include "math/Vectors.hpp"
#include "imgui.h"

namespace Overlay::ESP {
    extern Camera camera;

    void updateScreenSize();

    Vec3 worldToScreen(Vec3 worldPos);

    void drawPoint(Vec3 pos, ImU32 color);

    void drawLine(Vec3 start, Vec3 end, ImU32 color);

    void drawBox(Vec3 center, Vec3 size, ImU32 color);

    void drawText(Vec3 pos, std::string text, ImU32 color);

    void drawCircle(Vec3 center, float radius, ImU32 color, bool perspective = true, bool filled = false);

    void beginESPWindow(const char * name);

    void endESPWindow();
}