#pragma once
#include "math/Vectors.hpp"

namespace Mod::Mario::Coordinates {
    extern const float maxSafeMarioCoord;
    
    extern const float scaleFactor;
    extern const float invScaleFactor;

    // Runtime-tunable size of a chunk along one axis, in mario units.
    // libsm64 stores surface coords as int16, giving a hard limit of +/-32768.
    // Default to the full int16 range; lower it at runtime to stress-test chunk transitions.
    extern float chunkExtent;
    float chunkHalfExtent();

    bool isHaloVectorSafe(const Vec3 &haloCoords);
    Vec3 haloToMario(const Vec3& haloCoords);
    Vec3 marioToHalo(const Vec3& marioCoords);
    Vec3 marioToHalo(const int32_t* marioCoords);

    Vec3 marioDirectionToHalo(const Vec3& marioDirection);

    // Chunk helpers. Chunk (0,0,0) is centered on the origin and spans
    // [-chunkHalfExtent, +chunkHalfExtent) along each axis.
    Vec3i marioChunkForPosition(const Vec3& marioWorldPos);
    Vec3  marioChunkOrigin(const Vec3i& chunk);
    Vec3  marioWorldToLocal(const Vec3& worldMarioPos, const Vec3i& chunk);
    Vec3  marioLocalToWorld(const Vec3& localMarioPos, const Vec3i& chunk);
    Vec3i haloPosToMarioChunk(const Vec3& haloPos);
    Vec3  marioLocalToHaloWorld(const Vec3& localMario, const Vec3i& chunk);
    Vec3  marioLocalToHaloWorld(const int32_t* localMario, const Vec3i& chunk);
    Vec3 haloToMarioLocal(const Vec3 &haloPos, const Vec3i &chunk);

    extern const float maxSafeMarioCoord;
}