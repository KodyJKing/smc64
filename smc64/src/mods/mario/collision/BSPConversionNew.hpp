#pragma once
#include <vector>
#include <unordered_set>
#include "engine/halo1.hpp"
#include "../Coordinates.hpp"
#include "decomp/surface_terrains.h"
#include "libsm64.h"
#include "ClipTriangle.hpp"

#include <chrono>

#define PROFILE_BSP_CONVERSION 1

#define TWO_SIDED_MARGIN 10.0f

namespace Mod::Mario::BSPConversion {

    struct BSPConversionRequest {
        Engine::CollisionBSP* bsp;
        uint16_t surfaceType;
        BoundingBox* boundingBox;
        Vec3 offset;
    };

    std::vector<SM64Surface> convertBSPFast(BSPConversionRequest &request);

    std::vector<SM64Surface> convertBSP(BSPConversionRequest &request);
}
