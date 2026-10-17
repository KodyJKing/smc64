#include "Mario.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "spark/overlay/ESP.hpp"

#include "libsm64.h"

#include "MarioState.hpp"
#include "MarioModel.hpp"
#include "MarioSkeleton.hpp"
#include "MarioMelee.hpp"
#include "collision/MarioBSPChunk.hpp"
#include "collision/DynamicGeometry.hpp"
#include "collision/MarioCollisionDebugRender.hpp"
#include "Coordinates.hpp"
#include "MarioChiefPose.hpp"

// #define DEBUG_MARIO_GEOMETRY 1

namespace Mod::Mario {

    int32_t highlightTriangleIndex = -1;

    void marioDebugWindow() {
        ImGui::Begin("Mario Debug Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        std::string strMarioInControl = marioInControl() ? "Yes" : "No";
        ImGui::Text("Mario in control: %s", strMarioInControl.c_str());

        ImGui::Text("Mario Model Handle: %X", MarioModel::marioHandle);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy");
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) {
            char buffer[32];
            snprintf(buffer, sizeof(buffer), "%X", MarioModel::marioHandle);
            ImGui::SetClipboardText(buffer);
        }

        ImGui::Text("Mario ID: %d", marioId);
        ImGui::Text("Position: (%.2f, %.2f, %.2f)", marioState.position[0], marioState.position[1], marioState.position[2]);
        ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", marioState.velocity[0], marioState.velocity[1], marioState.velocity[2]);
        {
            Vec3i chunk  = marioChunk;
            Vec3i loaded = MarioBSPChunk::getLoadedChunk();
            Vec3  world  = marioWorldPosition();
            ImGui::Text("Chunk: (%d, %d, %d)  Loaded: (%d, %d, %d)",
                chunk.x, chunk.y, chunk.z, loaded.x, loaded.y, loaded.z);
            ImGui::Text("World pos (halo): (%.2f, %.2f, %.2f)", world.x, world.y, world.z);
            ImGui::SliderFloat("chunkExtent",  &Coordinates::chunkExtent,  1024.0f, 32768.0f, "%.0f");
            ImGui::SliderFloat("reloadMargin", &MarioBSPChunk::reloadMargin, 0.0f,  16384.0f, "%.0f");
        }
        ImGui::Text("Health: %d", marioState.health);
        ImGui::Text("Action: 0x%X", marioState.action);
        ImGui::Text("Flags: 0x%X", marioState.flags);

        #ifdef DEBUG_MARIO_COLLISION
        CollisionDebugRender::renderImGuiSection();
        #endif // DEBUG_MARIO_COLLISION

        // Show last_mario_action global.
        uint32_t lastMarioAction = Engine::Scripting::readGlobal("last_mario_action");
        ImGui::Text("Last Mario Action: 0x%X", lastMarioAction);

        ImGui::End();
    }

    void debugRender() {
#ifdef ENABLE_MARIO

        if (marioId < 0) return;

        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        #ifdef DEBUG_MARIO_GEOMETRY
        bool highlightEnabled = highlightTriangleIndex >= 0 && highlightTriangleIndex < static_cast<int32_t>(marioGeometry.numTrianglesUsed);
        for (int i = 0; i < marioGeometry.numTrianglesUsed; i++) {
            Vec3* pos = (Vec3*)&marioGeometry.position[i * 3 * 3];
            Vec3* color = (Vec3*)&marioGeometry.color[i * 3 * 3];

            bool highlightThis = highlightEnabled && i == highlightTriangleIndex;
            uint8_t alpha = 20;

            for (int j = 0; j < 3; j++) {
                Vec3 p1 = pos[j];
                Vec3 p2 = pos[(j + 1) % 3];

                Vec3 haloP1 = Coordinates::marioLocalToHaloWorld(p1, marioChunk);
                Vec3 haloP2 = Coordinates::marioLocalToHaloWorld(p2, marioChunk);

                ImU32 colorIm = IM_COL32(
                    static_cast<uint8_t>(color[j].x * 255),
                    static_cast<uint8_t>(color[j].y * 255),
                    static_cast<uint8_t>(color[j].z * 255),
                    alpha
                );

                Spark::Overlay::ESP::drawLine(haloP1, haloP2, colorIm);
            }

            if (highlightThis) {
                Vec3 haloP1 = Coordinates::marioLocalToHaloWorld(pos[0], marioChunk);
                Vec3 haloP2 = Coordinates::marioLocalToHaloWorld(pos[1], marioChunk);
                Vec3 haloP3 = Coordinates::marioLocalToHaloWorld(pos[2], marioChunk);
                Spark::Overlay::ESP::drawPoint(haloP1, IM_COL32(255, 0, 0, 255));
                Spark::Overlay::ESP::drawPoint(haloP2, IM_COL32(0, 255, 0, 255));
                Spark::Overlay::ESP::drawPoint(haloP3, IM_COL32(0, 0, 255, 255));
            }
        }
        #endif // DEBUG_MARIO_GEOMETRY

        #ifdef DEBUG_MARIO_COLLISION
        CollisionDebugRender::render();
        #endif // DEBUG_MARIO_COLLISION

        #ifdef DEBUG_MARIO_GEOMETRY
        {
            constexpr float AXIS_LENGTH = 30.0f;
            for (int i = 0; i < SM64_MARIO_BONE_COUNT; i++) {
                const SM64Matrix4f& m = marioBoneMatrices[i];
                Vec3 origin = { m.m[3][0], m.m[3][1], m.m[3][2] };
                Vec3 xEnd   = { origin.x + AXIS_LENGTH * m.m[0][0], origin.y + AXIS_LENGTH * m.m[0][1], origin.z + AXIS_LENGTH * m.m[0][2] };
                Vec3 yEnd   = { origin.x + AXIS_LENGTH * m.m[1][0], origin.y + AXIS_LENGTH * m.m[1][1], origin.z + AXIS_LENGTH * m.m[1][2] };
                Vec3 zEnd   = { origin.x + AXIS_LENGTH * m.m[2][0], origin.y + AXIS_LENGTH * m.m[2][1], origin.z + AXIS_LENGTH * m.m[2][2] };
                Vec3 wOrigin = Coordinates::marioLocalToHaloWorld(origin, marioChunk);
                Vec3 wXEnd   = Coordinates::marioLocalToHaloWorld(xEnd,   marioChunk);
                Vec3 wYEnd   = Coordinates::marioLocalToHaloWorld(yEnd,   marioChunk);
                Vec3 wZEnd   = Coordinates::marioLocalToHaloWorld(zEnd,   marioChunk);
                Spark::Overlay::ESP::drawLine(wOrigin, wXEnd, IM_COL32(255,   0,   0, 255));
                Spark::Overlay::ESP::drawLine(wOrigin, wYEnd, IM_COL32(  0, 255,   0, 255));
                Spark::Overlay::ESP::drawLine(wOrigin, wZEnd, IM_COL32(  0,   0, 255, 255));
            }
        }
        #endif // DEBUG_MARIO_GEOMETRY

        MarioMelee::debugRender();
        DynamicGeometry::debugRender();
        MarioBSPChunk::debugRender();
        MarioChiefPose::render();

        marioDebugWindow();

#endif // ENABLE_MARIO
    }

}
