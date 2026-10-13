#include <vector>
#include <unordered_set>
#include "engine/halo1.hpp"
#include "../Coordinates.hpp"
#include "decomp/surface_terrains.h"
#include "libsm64.h"
#include "ClipTriangle.hpp"
#include "BSPConversionNew.hpp"

#include "spark/overlay/Gizmos.hpp"

#include <chrono>

#define PROFILE_BSP_CONVERSION 1

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
        #ifdef PROFILE_BSP_CONVERSION
        auto startTime = std::chrono::high_resolution_clock::now();
        #endif // PROFILE_BSP_CONVERSION
        
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

        #define USE_FAST 1
        #if USE_FAST
        // Convert to halo coords.
        boundingBox.min = Coordinates::marioToHalo(boundingBox.min);
        boundingBox.max = Coordinates::marioToHalo(boundingBox.max);
        Spark::Overlay::Gizmos::drawBox(boundingBox.min, boundingBox.max, 0xFFFFFFFF, 60);
        auto surfaces = BSPConversion::convertBSPFast(request);
        #else
        auto surfaces = BSPConversion::convertBSP(request);
        #endif // USE_FAST

        #ifdef PROFILE_BSP_CONVERSION
        auto endTime = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
        printf("BSP conversion took %lld microseconds\n", duration);
        #endif // PROFILE_BSP_CONVERSION

        return surfaces;
    }

}
