#include "ESP.hpp"

namespace Overlay::ESP {

    Camera camera;

    struct Line {
        Vec3 a, b;

        // Clip line to lie in positive z
        Line clip() {
            if (a.z < 0 && b.z < 0) {
                return Line{Vec3{0, 0, 0}, Vec3{0, 0, 0}};
            }

            if (a.z < 0) {
                Vec3 intercept = a + (b - a) * (a.z / (a.z - b.z));
                return Line{intercept, b};
            }

            if (b.z < 0) {
                Vec3 intercept = a + (b - a) * (a.z / (a.z - b.z));
                return Line{a, intercept};
            }

            return *this;
        }
    };

    void updateScreenSize() {
        ImGuiIO& io = ImGui::GetIO();
        camera.width = io.DisplaySize.x;
        camera.height = io.DisplaySize.y;
    }

    Vec3 worldToScreen(Vec3 worldPos) {
        return camera.project(worldPos);
    }

    void drawPoint(Vec3 pos, ImU32 color) {
        Vec3 screenPos = worldToScreen(pos);

        if (screenPos.z < 0)
            return;

        ImGui::GetWindowDrawList()->AddRect(
            ImVec2{screenPos.x - 1.0f, screenPos.y - 1.0f},
            ImVec2{screenPos.x + 1.0f, screenPos.y + 1.0f},
            color
        );
    }

    void drawLine(Vec3 a, Vec3 b, ImU32 color) {
        Vec3 screenA = worldToScreen(a);
        Vec3 screenB = worldToScreen(b);

        if (screenA.z < 0 && screenB.z < 0)
            return;

        Line line = Line{screenA, screenB}.clip();

        ImGui::GetWindowDrawList()->AddLine(
            ImVec2{line.a.x, line.a.y},
            ImVec2{line.b.x, line.b.y},
            color
        );
    }

    void drawBox(Vec3 center, Vec3 size, ImU32 color) {
        Vec3 min = center - size / 2;
        Vec3 max = center + size / 2;

        Vec3 corners[8] = {
            Vec3{min.x, min.y, min.z},
            Vec3{min.x, min.y, max.z},
            Vec3{min.x, max.y, min.z},
            Vec3{min.x, max.y, max.z},
            Vec3{max.x, min.y, min.z},
            Vec3{max.x, min.y, max.z},
            Vec3{max.x, max.y, min.z},
            Vec3{max.x, max.y, max.z}
        };

        for (int i = 0; i < 4; i++) {
            drawLine(corners[i], corners[i + 4], color);
        }

        for (int i = 0; i < 3; i++) {
            drawLine(corners[i], corners[i + 1], color);
            drawLine(corners[i + 4], corners[i + 5], color);
            drawLine(corners[i], corners[i + 2], color);
        }

        drawLine(corners[3], corners[0], color);
        drawLine(corners[7], corners[4], color);
        drawLine(corners[3], corners[7], color);
    }

    void drawText(Vec3 pos, std::string text, ImU32 color) {
        Vec3 screenPos = worldToScreen(pos);

        if (screenPos.z < 0)
            return;

        ImGui::GetWindowDrawList()->AddText(
            ImVec2{screenPos.x, screenPos.y},
            color,
            text.c_str()
        );
    }

    void drawCircle(Vec3 center, float radius, ImU32 color, bool perspective, bool filled) {
        Vec3 screenCenter = worldToScreen(center);

        if (screenCenter.z < 0)
            return;

        float screenRadius = radius;
        if (perspective) {
            Vec3 screenRadiusVec = worldToScreen(center + camera.up * radius) - screenCenter;
            screenRadius = screenRadiusVec.length();
        }

        if (filled) {
            ImGui::GetWindowDrawList()->AddCircleFilled(
                ImVec2{screenCenter.x, screenCenter.y},
                screenRadius,
                color
            );
        } else {
            ImGui::GetWindowDrawList()->AddCircle(
                ImVec2{screenCenter.x, screenCenter.y},
                screenRadius,
                color
            );
        }
    }

    void beginESPWindow(const char * name) {
        ESP::updateScreenSize();
        ImGui::Begin(
            name, 0,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove |
                ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoScrollbar |
                ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse |
                ImGuiWindowFlags_NoNav | ImGuiWindowFlags_NoBringToFrontOnFocus |
                ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMouseInputs
        );
        // Set Window position to top left corner
        ImGui::SetWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        // Set Window size to full screen
        ImGui::SetWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);
    }

    void endESPWindow() {
        ImGui::End();
    }

}