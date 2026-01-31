#include "Coordinates.hpp"

namespace HaloCE::Mod::Coordinates {

    const float scaleFactor = 400.0f;
    const float invScaleFactor = 1.0f / scaleFactor;

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
}
