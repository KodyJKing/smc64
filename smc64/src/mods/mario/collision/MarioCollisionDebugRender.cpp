#include "MarioCollisionDebugRender.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "spark/overlay/ESP.hpp"

#include "libsm64.h"

#include "../MarioState.hpp"
#include "../Mario.hpp"
#include "../Coordinates.hpp"

namespace Mod::Mario {

#ifdef DEBUG_MARIO_COLLISION

namespace {

    ImU32 withAlpha(ImU32 color, uint8_t alpha) {
        return (color & 0x00FFFFFFu) | ((ImU32)alpha << 24);
    }

    void draw_SM64SurfaceCollisionData(SM64SurfaceCollisionData* surfaceData, ImU32 color, int label = -1, Vec3* marioPos = nullptr) {
        namespace ESP = Spark::Overlay::ESP;
        Camera& camera = ESP::camera;
        Vec3 v1 = Coordinates::marioLocalToHaloWorld(surfaceData->vertex1, marioChunk);
        Vec3 v2 = Coordinates::marioLocalToHaloWorld(surfaceData->vertex2, marioChunk);
        Vec3 v3 = Coordinates::marioLocalToHaloWorld(surfaceData->vertex3, marioChunk);
        ESP::drawLine(v1, v2, color);
        ESP::drawLine(v2, v3, color);
        ESP::drawLine(v3, v1, color);

        Vec3 center = (v1 + v2 + v3) / 3.0f;
        auto n = surfaceData->normal;
        Vec3 normalEnd = center + Vec3{n.x, n.z, n.y} * 0.1f;
        ESP::drawLine(center, normalEnd, color);
        ESP::drawPoint(center, color);

        if (marioPos) {
            uint8_t lineAlpha = static_cast<uint8_t>(((color >> 24) & 0xFF) / 4);
            ESP::drawLine(center, *marioPos, withAlpha(color, lineAlpha));
            ESP::drawPoint(*marioPos, color);
        }

        if (label >= 0)
            ESP::drawText(center, std::to_string(label).c_str(), color);
    }

    // Current-frame state exposed to renderImGuiSection
    SM64SurfaceCollisionData s_currentFloor = {};
    bool s_hasFloor = false;
    int16_t s_numWalls = 0;

    #ifdef DEBUG_MARIO_COLLISION_HISTORY

    constexpr int32_t MARIO_POSITION_TRAIL_SIZE = 128;
    constexpr float   MARIO_POSITION_TRAIL_MIN_DIST = 0.05f; // Halo world units

    struct SurfaceSnapshot {
        bool valid = false;
        SM64SurfaceCollisionData data = {};
        int32_t seqNum = -1;
        Vec3 marioPos = {};
    };

    struct WallSetSnapshot {
        int16_t numWalls = 0;
        SM64SurfaceCollisionData walls[4] = {};
        int32_t seqNum = -1;
        Vec3 marioPos = {};
    };

    SurfaceSnapshot s_floorHistory[COLLISION_HISTORY_SIZE];
    SurfaceSnapshot s_ceilHistory[COLLISION_HISTORY_SIZE];
    WallSetSnapshot s_wallHistory[COLLISION_HISTORY_SIZE];
    int32_t s_floorHead = 0, s_floorCount = 0;
    int32_t s_ceilHead  = 0, s_ceilCount  = 0;
    int32_t s_wallHead  = 0, s_wallCount  = 0;
    int32_t s_changeCounter = 0;

    Vec3    s_posTrail[MARIO_POSITION_TRAIL_SIZE];
    int32_t s_posTrailHead  = 0;
    int32_t s_posTrailCount = 0;

    bool surfacesEqual(const SM64SurfaceCollisionData& a, const SM64SurfaceCollisionData& b) {
        return memcmp(a.vertex1, b.vertex1, sizeof(a.vertex1)) == 0
            && memcmp(a.vertex2, b.vertex2, sizeof(a.vertex2)) == 0
            && memcmp(a.vertex3, b.vertex3, sizeof(a.vertex3)) == 0;
    }

    bool snapshotEqual(const SurfaceSnapshot& a, const SurfaceSnapshot& b) {
        if (a.valid != b.valid) return false;
        if (!a.valid) return true;
        return surfacesEqual(a.data, b.data);
    }

    bool wallSetsEqual(const WallSetSnapshot& a, const WallSetSnapshot& b) {
        if (a.numWalls != b.numWalls) return false;
        for (int i = 0; i < a.numWalls; i++) {
            if (!surfacesEqual(a.walls[i], b.walls[i])) return false;
        }
        return true;
    }

    void pushSurfaceHistory(SurfaceSnapshot* history, int32_t& head, int32_t& count,
                            SM64SurfaceCollisionData* surface) {
        SurfaceSnapshot snap;
        snap.valid = (surface != nullptr);
        if (surface) snap.data = *surface;
        if (count > 0) {
            int newestIdx = (head - 1 + COLLISION_HISTORY_SIZE) % COLLISION_HISTORY_SIZE;
            if (snapshotEqual(history[newestIdx], snap)) return;
        }
        snap.seqNum = s_changeCounter++;
        snap.marioPos = marioWorldPosition();
        history[head] = snap;
        head = (head + 1) % COLLISION_HISTORY_SIZE;
        if (count < COLLISION_HISTORY_SIZE) count++;
    }

    void pushWallHistory(SM64WallCollisionData* wallData) {
        WallSetSnapshot snap;
        snap.numWalls = wallData->numWalls;
        for (int i = 0; i < wallData->numWalls; i++) {
            if (wallData->walls[i]) snap.walls[i] = *wallData->walls[i];
        }
        if (s_wallCount > 0) {
            int newestIdx = (s_wallHead - 1 + COLLISION_HISTORY_SIZE) % COLLISION_HISTORY_SIZE;
            if (wallSetsEqual(s_wallHistory[newestIdx], snap)) return;
        }
        snap.seqNum = s_changeCounter++;
        snap.marioPos = marioWorldPosition();
        s_wallHistory[s_wallHead] = snap;
        s_wallHead = (s_wallHead + 1) % COLLISION_HISTORY_SIZE;
        if (s_wallCount < COLLISION_HISTORY_SIZE) s_wallCount++;
    }

    void drawSurfaceHistory(SurfaceSnapshot* history, int32_t head, int32_t count, ImU32 color) {
        for (int age = count - 1; age >= 0; age--) {
            // age 0 = newest, age count-1 = oldest
            int idx = (head - 1 - age + COLLISION_HISTORY_SIZE * 2) % COLLISION_HISTORY_SIZE;
            SurfaceSnapshot& snap = history[idx];
            if (!snap.valid) continue;
            float t = (count <= 1) ? 1.0f : (float)(count - 1 - age) / (float)(count - 1);
            uint8_t alpha = static_cast<uint8_t>(30 + t * (255 - 30));
            SM64SurfaceCollisionData copy = snap.data;
            draw_SM64SurfaceCollisionData(&copy, withAlpha(color, alpha), snap.seqNum, &snap.marioPos);
        }
    }

    void pushPositionTrail(Vec3 pos) {
        if (s_posTrailCount > 0) {
            int newestIdx = (s_posTrailHead - 1 + MARIO_POSITION_TRAIL_SIZE) % MARIO_POSITION_TRAIL_SIZE;
            Vec3 d = pos - s_posTrail[newestIdx];
            if (d.x * d.x + d.y * d.y + d.z * d.z < MARIO_POSITION_TRAIL_MIN_DIST * MARIO_POSITION_TRAIL_MIN_DIST)
                return;
        }
        s_posTrail[s_posTrailHead] = pos;
        s_posTrailHead = (s_posTrailHead + 1) % MARIO_POSITION_TRAIL_SIZE;
        if (s_posTrailCount < MARIO_POSITION_TRAIL_SIZE) s_posTrailCount++;
    }

    void drawPositionTrail() {
        namespace ESP = Spark::Overlay::ESP;
        constexpr ImU32 trailColor = IM_COL32(255, 255, 255, 60);
        Vec3 prev{};
        bool hasPrev = false;
        for (int age = s_posTrailCount - 1; age >= 0; age--) {
            // age 0 = newest
            int idx = (s_posTrailHead - 1 - age + MARIO_POSITION_TRAIL_SIZE * 2) % MARIO_POSITION_TRAIL_SIZE;
            Vec3 p = s_posTrail[idx];
            ESP::drawPoint(p, trailColor);
            if (hasPrev)
                ESP::drawLine(prev, p, trailColor);
            prev    = p;
            hasPrev = true;
        }
    }

    void drawWallHistory() {
        for (int age = s_wallCount - 1; age >= 0; age--) {
            int idx = (s_wallHead - 1 - age + COLLISION_HISTORY_SIZE * 2) % COLLISION_HISTORY_SIZE;
            WallSetSnapshot& snap = s_wallHistory[idx];
            float t = (s_wallCount <= 1) ? 1.0f : (float)(s_wallCount - 1 - age) / (float)(s_wallCount - 1);
            uint8_t alpha = static_cast<uint8_t>(30 + t * (255 - 30));
            ImU32 faded = withAlpha(IM_COL32(0, 200, 255, 255), alpha);
            for (int i = 0; i < snap.numWalls; i++) {
                SM64SurfaceCollisionData copy = snap.walls[i];
                draw_SM64SurfaceCollisionData(&copy, faded, snap.seqNum, &snap.marioPos);
            }
        }
    }

    #endif // DEBUG_MARIO_COLLISION_HISTORY

} // anonymous namespace

namespace CollisionDebugRender {

    void render() {
        SM64SurfaceCollisionData* floorData = nullptr;
        SM64SurfaceCollisionData* ceilData  = nullptr;
        SM64WallCollisionData wallData = {};

        sm64_surface_find_floor(
            marioState.position[0],
            marioState.position[1],
            marioState.position[2],
            &floorData
        );

        sm64_surface_find_ceil(
            marioState.position[0],
            marioState.position[1],
            marioState.position[2],
            &ceilData
        );

        wallData.x       = marioState.position[0];
        wallData.y       = marioState.position[1];
        wallData.z       = marioState.position[2];
        wallData.radius  = 100.0f;
        wallData.offsetY = 10.0f;
        sm64_surface_find_wall_collisions(&wallData);

        // Store current-frame state for renderImGuiSection
        s_hasFloor = (floorData != nullptr);
        if (s_hasFloor) s_currentFloor = *floorData;
        s_numWalls = wallData.numWalls;

        #ifdef DEBUG_MARIO_COLLISION_HISTORY
        pushSurfaceHistory(s_floorHistory, s_floorHead, s_floorCount, floorData);
        pushSurfaceHistory(s_ceilHistory,  s_ceilHead,  s_ceilCount,  ceilData);
        pushWallHistory(&wallData);
        pushPositionTrail(marioWorldPosition());

        drawPositionTrail();
        drawSurfaceHistory(s_floorHistory, s_floorHead, s_floorCount, IM_COL32(255, 200, 0, 255));
        drawSurfaceHistory(s_ceilHistory,  s_ceilHead,  s_ceilCount,  IM_COL32(255, 0, 200, 255));
        drawWallHistory();
        #else
        if (floorData)
            draw_SM64SurfaceCollisionData(floorData, IM_COL32(255, 200, 0, 255));
        if (ceilData)
            draw_SM64SurfaceCollisionData(ceilData, IM_COL32(255, 0, 200, 255));
        for (int i = 0; i < wallData.numWalls; i++) {
            if (wallData.walls[i])
                draw_SM64SurfaceCollisionData(wallData.walls[i], IM_COL32(0, 200, 255, 255));
        }
        #endif // DEBUG_MARIO_COLLISION_HISTORY
    }

    void renderImGuiSection() {
        ImGui::Text("Num walls: %d", s_numWalls);
        if (s_hasFloor) {
            auto* v1 = s_currentFloor.vertex1;
            auto* v2 = s_currentFloor.vertex2;
            auto* v3 = s_currentFloor.vertex3;
            ImGui::Text("Floor v1: (%d, %d, %d)", v1[0], v1[1], v1[2]);
            ImGui::Text("Floor v2: (%d, %d, %d)", v2[0], v2[1], v2[2]);
            ImGui::Text("Floor v3: (%d, %d, %d)", v3[0], v3[1], v3[2]);
        } else {
            ImGui::Text("Floor: none");
        }
    }

} // namespace CollisionDebugRender

#endif // DEBUG_MARIO_COLLISION

} // namespace Mod::Mario
