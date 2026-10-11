#include "Coordinates.hpp"
#include <cmath>

namespace Mod::Mario::Coordinates {

    struct MarioCoords {
        Vec3 local;
        Vec3i chunk;
    };

    const float scaleFactor = 400.0f;
    const float invScaleFactor = 1.0f / scaleFactor;

    float chunkExtent = 32768.0f;
    float chunkHalfExtent() { return chunkExtent * 0.5f; }

    const float maxSafeMarioCoord = 50000.0f;
    const float maxSafeHaloCoord = maxSafeMarioCoord * invScaleFactor;

    bool isHaloVectorSafe(const Vec3& haloCoords) {
        if (abs(haloCoords.x) > maxSafeHaloCoord ||
            abs(haloCoords.y) > maxSafeHaloCoord ||
            abs(haloCoords.z) > maxSafeHaloCoord) {
            return false;
        }
        return true;
    }

    // Transform a Vec3 as a vector.
    Vec3 haloToMario(const Vec3& haloCoords) {
        // Convert Halo CE coordinates to Super Mario 64 coordinates
        return Vec3{ haloCoords.x * scaleFactor, haloCoords.z * scaleFactor, haloCoords.y * scaleFactor };
    }

    Vec3 marioToHalo(const Vec3& marioCoords) {
        // Convert Super Mario 64 coordinates to Halo CE coordinates
        return Vec3{ marioCoords.x * invScaleFactor, marioCoords.z * invScaleFactor, marioCoords.y * invScaleFactor };
    }

    Vec3 marioToHalo(const int32_t* marioCoords) {
        // Convert Super Mario 64 coordinates to Halo CE coordinates
        return Vec3{ marioCoords[0] * invScaleFactor, marioCoords[2] * invScaleFactor, marioCoords[1] * invScaleFactor };
    }

    Vec3 marioDirectionToHalo(const Vec3 &marioDirection) {
        // Convert a direction vector from Super Mario 64 coordinates to Halo CE coordinates
        return Vec3{ marioDirection.x, marioDirection.z, marioDirection.y };
    }

    Vec3i marioChunkForPosition(const Vec3& marioWorldPos) {
        float inv = 1.0f / chunkExtent;
        return Vec3i{
            (int32_t) std::lround(marioWorldPos.x * inv),
            (int32_t) std::lround(marioWorldPos.y * inv),
            (int32_t) std::lround(marioWorldPos.z * inv),
        };
    }

    Vec3 marioChunkOrigin(const Vec3i& chunk) {
        return Vec3{
            (float) chunk.x * chunkExtent,
            (float) chunk.y * chunkExtent,
            (float) chunk.z * chunkExtent,
        };
    }

    Vec3 marioWorldToLocal(const Vec3& worldMarioPos, const Vec3i& chunk) {
        Vec3 o = marioChunkOrigin(chunk);
        return Vec3{ worldMarioPos.x - o.x, worldMarioPos.y - o.y, worldMarioPos.z - o.z };
    }

    Vec3 marioLocalToWorld(const Vec3& localMarioPos, const Vec3i& chunk) {
        Vec3 o = marioChunkOrigin(chunk);
        return Vec3{ localMarioPos.x + o.x, localMarioPos.y + o.y, localMarioPos.z + o.z };
    }

    Vec3i haloPosToMarioChunk(const Vec3& haloPos) {
        return marioChunkForPosition(haloToMario(haloPos));
    }

    // Transform a Vec3 as a point.
    Vec3 marioLocalToHaloWorld(const Vec3& localMario, const Vec3i& chunk) {
        return marioToHalo(marioLocalToWorld(localMario, chunk));
    }

    Vec3 marioLocalToHaloWorld(const int32_t* localMario, const Vec3i& chunk) {
        Vec3 v = { (float)localMario[0], (float)localMario[1], (float)localMario[2] };
        return marioLocalToHaloWorld(v, chunk);
    }

    Vec3 haloToMarioLocal(const Vec3& haloPos, const Vec3i& chunk) {
        Vec3 marioPos = haloToMario(haloPos);
        return marioWorldToLocal(marioPos, chunk);
    }
}
