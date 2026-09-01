#pragma once

#include "math/Vectors.hpp"

namespace Mod::Mario {
    // Create a temporary square floor in mario-local space to support spawn.
    // This is replaced with real BSP geometry immediately after by MarioBSPChunk::init.
    void createSpawnPlatform(const Vec3& localPos);
}
