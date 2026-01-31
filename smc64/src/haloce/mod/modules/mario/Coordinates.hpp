#pragma once
#include "math/Vectors.hpp"

namespace HaloCE::Mod::Coordinates {
    Vec3 haloToMario(const Vec3& haloCoords);
    Vec3 marioToHalo(const Vec3& marioCoords);
    Vec3 marioToHalo(const int32_t* marioCoords);
}