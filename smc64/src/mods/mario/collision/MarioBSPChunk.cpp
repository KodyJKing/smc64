#include "MarioBSPChunk.hpp"

#include "libsm64.h"

#include "BSPConversion.hpp"
#include "DynamicGeometry.hpp"
#include "MarioFalseFloor.hpp"

#include "../MarioState.hpp"
#include "../Coordinates.hpp"
#include "../level-edit/MarioLevelEdit.hpp"

#include <cstdlib>
#include <cstdio>
#include <cmath>
#include <cstring>

// #define DEBUG_BSP_CHUNK 1

#ifdef DEBUG_BSP_CHUNK
    // #define MAX_DISTANCE_WORLD_UNITS 2.0f
    #define MAX_DISTANCE_WORLD_UNITS 100.0f
    #include <iostream>
    #include "spark/overlay/ESP.hpp"
    #define LOG(x) std::cout << "[MarioBSPChunk] " << x << std::endl;
#else
    #define LOG(x) ;
#endif

namespace Mod::Mario::MarioBSPChunk {

    // Mario's chunk is the frame all of sm64's coords (static surfaces, dynamic objects,
    // marioState.position) are expressed in. We allow Mario's local position to drift up
    // to chunkHalfExtent + reloadMargin from the chunk origin before performing an
    // atomic recenter + static reload.
    float reloadMargin = 4096.0f / 4.0f;

    static SM64Surface* staticSurfaces    = nullptr;
    static size_t       staticSurfacesCount = 0;

    static uint64_t lastBSPSignature = 0;

    Vec3i getLoadedChunk() { return marioChunk; }

    static void uploadFor(Vec3i chunk) {
        auto surfaces = Mod::Mario::BSPConversion::haloGeometryToMarioForChunk(chunk, 1);

        auto levelEditSurfaces = ::Mod::Mario::LevelEdit::getStaticSurfaces(chunk);
        surfaces.insert(surfaces.end(), levelEditSurfaces.begin(), levelEditSurfaces.end());
        if (staticSurfaces) {
            ::free(staticSurfaces);
            staticSurfaces = nullptr;
            staticSurfacesCount = 0;
        }

        staticSurfacesCount = surfaces.size();
        staticSurfaces = (SM64Surface*) malloc(sizeof(SM64Surface) * staticSurfacesCount);
        memcpy(staticSurfaces, surfaces.data(), sizeof(SM64Surface) * staticSurfacesCount);

        sm64_static_surfaces_load(staticSurfaces, (uint32_t) staticSurfacesCount);
        LOG("Loaded " << staticSurfacesCount << " surfaces for chunk (" << chunk.x << ", " << chunk.y << ", " << chunk.z << ")");

        lastBSPSignature = Engine::getBSPSignature();
    }

    void init(Vec3i initialChunk) {
        marioChunk = initialChunk;
        uploadFor(marioChunk);
    }

    void reloadFor(Vec3i newChunk) {
        Vec3i oldChunk = marioChunk;
        marioChunk     = newChunk;
        uploadFor(marioChunk);
        Mod::Mario::DynamicGeometry::onLoadedChunkChanged(oldChunk, newChunk);
    }

    void checkMovement() {
        if (marioId < 0) return;

        float ext       = Coordinates::chunkExtent;
        float half      = Coordinates::chunkHalfExtent();
        float threshold = half + reloadMargin;

        float* p = marioState.position;
        Vec3i delta = { 0, 0, 0 };
        if (std::abs(p[0]) > threshold) delta.x = (int32_t) std::lround(p[0] / ext);
        if (std::abs(p[1]) > threshold) delta.y = (int32_t) std::lround(p[1] / ext);
        if (std::abs(p[2]) > threshold) delta.z = (int32_t) std::lround(p[2] / ext);
        if (delta.x == 0 && delta.y == 0 && delta.z == 0) return;

        // Atomic recenter: shift Mario's sm64 coord and reload static + dynamic geometry
        // for the new chunk, so the static collision world stays beneath Mario.
        Vec3i newChunk = { marioChunk.x + delta.x, marioChunk.y + delta.y, marioChunk.z + delta.z };
        p[0] -= (float) delta.x * ext;
        p[1] -= (float) delta.y * ext;
        p[2] -= (float) delta.z * ext;
        sm64_set_mario_position(marioId, p[0], p[1], p[2]);

        LOG("Mario moved to chunk (" << newChunk.x << ", " << newChunk.y << ", " << newChunk.z
            << ") with local position (" << p[0] << ", " << p[1] << ", " << p[2] << ")");
        reloadFor(newChunk);
    }

    void checkSignature() {
        uint64_t currentBSPSignature = Engine::getBSPSignature();
        if (currentBSPSignature != lastBSPSignature) {
            LOG("BSP signature changed, reloading Mario's static surfaces for chunk ("
                << marioChunk.x << ", " << marioChunk.y << ", " << marioChunk.z << ")");
            reloadFor(marioChunk);
        }
    }

    void maintain() {
        checkMovement();
        checkSignature();
        Mod::Mario::updateFalseFloor();
    }

    void free() {
        if (staticSurfaces) {
            ::free(staticSurfaces);
            staticSurfaces = nullptr;
            staticSurfacesCount = 0;
        }
        Mod::Mario::freeFalseFloor();
    }

    bool hasValidLoadedChunk() {
        return staticSurfaces != nullptr;
    }

    #ifdef DEBUG_BSP_CHUNK
    static void drawStaticSurfaces() {
        namespace ESP = Spark::Overlay::ESP;
        auto cameraPos = ESP::camera.pos;
        constexpr ImU32 color = IM_COL32(0, 255, 128, 80);
        for (size_t i = 0; i < staticSurfacesCount; i++) {
            const SM64Surface& s = staticSurfaces[i];
            Vec3 v0 = Coordinates::marioLocalToHaloWorld(s.vertices[0], marioChunk);
            Vec3 v1 = Coordinates::marioLocalToHaloWorld(s.vertices[1], marioChunk);
            Vec3 v2 = Coordinates::marioLocalToHaloWorld(s.vertices[2], marioChunk);

            if ((v0 - cameraPos).length() > MAX_DISTANCE_WORLD_UNITS &&
                (v1 - cameraPos).length() > MAX_DISTANCE_WORLD_UNITS &&
                (v2 - cameraPos).length() > MAX_DISTANCE_WORLD_UNITS) {
                continue;
            }
            
            ESP::drawLine(v0, v1, color);
            ESP::drawLine(v1, v2, color);
            ESP::drawLine(v2, v0, color);
            
            Vec3 normal = (v2 - v0).cross(v1 - v0).normalize();
            Vec3 center = (v0 + v1 + v2) / 3.0f;
            Vec3 normalEnd = center + normal * 0.025f;
            
            const float wallThreshold = 0.2f;
            bool isWall = abs(normal.z) < wallThreshold;
            auto color = isWall ? IM_COL32(255, 0, 0, 255) : IM_COL32(0, 255, 0, 255);
            
            ESP::drawLine(center, normalEnd, color);
            ESP::drawPoint(center, color);
        }
    }

    static void drawChunkBounds() {
        namespace ESP = Spark::Overlay::ESP;
        constexpr ImU32 color = IM_COL32(0, 128, 255, 80);

        float ext = Coordinates::chunkExtent;
        float half = ext / 2.0f;

        Vec3 marioLocalMin = Vec3{-half, -half, -half};
        Vec3 marioLocalMax = Vec3{half, half, half};

        Vec3 minBound = Coordinates::marioLocalToHaloWorld(marioLocalMin, marioChunk);
        Vec3 maxBound = Coordinates::marioLocalToHaloWorld(marioLocalMax, marioChunk);

        Vec3 center = Coordinates::marioLocalToHaloWorld(Vec3{0, 0, 0}, marioChunk);
        Vec3 size = maxBound - minBound;

        ESP::drawBox(center, size, color);
    }

    #endif // DEBUG_BSP_CHUNK

    void debugRender() {
        #ifdef DEBUG_BSP_CHUNK
        drawStaticSurfaces();
        // drawChunkBounds();
        #endif
    }

}
