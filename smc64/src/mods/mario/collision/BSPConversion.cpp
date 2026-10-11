#include <vector>
#include <unordered_set>
#include "engine/halo1.hpp"
#include "../Coordinates.hpp"
#include "decomp/surface_terrains.h"
#include "libsm64.h"
#include "ClipTriangle.hpp"
#include "BSPConversionNew.hpp"

#include <chrono>

#define PROFILE_BSP_CONVERSION 1

#define TWO_SIDED_MARGIN 10.0f

namespace Mod::Mario::BSPConversion {

    std::vector<SM64Surface> convertBSP(Engine::CollisionBSP* bsp, uint16_t defaultSurfaceType) {
        BSPConversionRequest request;
        
        request.bsp = bsp;
        request.offset = Vec3{0, 0, 0};
        request.surfaceType = defaultSurfaceType;
        
        BoundingBox boundingBox;
        float ext = Coordinates::maxSafeMarioCoord;
        boundingBox.min = Vec3{-ext, -ext, -ext};
        boundingBox.max = Vec3{ext, ext, ext};
        request.boundingBox = &boundingBox;
        
        return convertBSP(request);
    }

    // Convert Halo CE static world geometry into mario-space surfaces for the chunk
    // neighborhood centered on `loadedChunk` (radius=1 -> 3x3x3 chunks).
    std::vector<SM64Surface> haloGeometryToMarioForChunk(Vec3i loadedChunk, int radius) {
        // Neighborhood AABB in mario world space.
        
        BSPConversion::BSPConversionRequest request;
        
        request.bsp = (Engine::CollisionBSP*) Engine::getBSPPointer();
        
        Vec3 chunkOriginMario = Coordinates::marioChunkOrigin(loadedChunk);

        BoundingBox boundingBox;
        float halfExtent = Coordinates::chunkHalfExtent();
        float reach      = (float) radius * Coordinates::chunkExtent + halfExtent;
        boundingBox.min = { chunkOriginMario.x - reach, chunkOriginMario.y - reach, chunkOriginMario.z - reach };
        boundingBox.max = { chunkOriginMario.x + reach, chunkOriginMario.y + reach, chunkOriginMario.z + reach };
        request.boundingBox = &boundingBox;
        
        request.offset = chunkOriginMario * -1.0f;

        auto surfaces = BSPConversion::convertBSP(request);

        return surfaces;
    }

}
