#pragma once

#include <vector>
#include <cstdint>
#include "libsm64.h"
#include "engine/halo1.hpp"
#include "math/Vectors.hpp"

namespace Mod::Mario::BSPConversion {

    // Convert a Halo CE CollisionBSP to an array of SM64Surface.
    // Vertices are converted to mario space and stored as-is (no chunk translation).
    // Used for dynamic (object-local) collision data.
    std::vector<SM64Surface> convertBSP(Engine::CollisionBSP* bsp, uint16_t defaultSurfaceType);

    // Convert Halo CE static world geometry into mario-space surfaces for the chunk
    // neighborhood centered on `loadedChunk` (radius = 1 -> 3x3x3 chunks).
    // Triangles whose vertices are all outside the neighborhood AABB are dropped.
    // Kept vertices are translated by -Coordinates::marioChunkOrigin(loadedChunk) so the
    // resulting surfaces are expressed in chunk-local mario coordinates.
    std::vector<SM64Surface> haloGeometryToMarioForChunk(Vec3i loadedChunk, int radius = 1);

}
