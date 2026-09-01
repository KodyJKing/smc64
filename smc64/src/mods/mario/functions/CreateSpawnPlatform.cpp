#include "CreateSpawnPlatform.hpp"

#include "libsm64.h"
#include "decomp/surface_terrains.h"

#include <cmath>
#include <cstdint>

namespace Mod::Mario {

    void createSpawnPlatform(const Vec3& localPos) {
        int32_t fx = (int32_t)std::lround(localPos.x);
        int32_t fz = (int32_t)std::lround(localPos.z);
        int32_t fy = (int32_t)std::lround(localPos.y) - 200; // ~0.5m below spawn
        int32_t half = 2000; // ~5m half-extent
        SM64Surface floor[2] = {};
        for (auto& s : floor) { s.type = SURFACE_DEFAULT; s.force = 0; s.terrain = TERRAIN_GRASS; }
        // CCW winding (viewed from above) for upward normal
        floor[0].vertices[0][0] = fx - half; floor[0].vertices[0][1] = fy; floor[0].vertices[0][2] = fz - half;
        floor[0].vertices[1][0] = fx - half; floor[0].vertices[1][1] = fy; floor[0].vertices[1][2] = fz + half;
        floor[0].vertices[2][0] = fx + half; floor[0].vertices[2][1] = fy; floor[0].vertices[2][2] = fz + half;
        floor[1].vertices[0][0] = fx - half; floor[1].vertices[0][1] = fy; floor[1].vertices[0][2] = fz - half;
        floor[1].vertices[1][0] = fx + half; floor[1].vertices[1][1] = fy; floor[1].vertices[1][2] = fz + half;
        floor[1].vertices[2][0] = fx + half; floor[1].vertices[2][1] = fy; floor[1].vertices[2][2] = fz - half;
        sm64_static_surfaces_load(floor, 2);
    }

}
