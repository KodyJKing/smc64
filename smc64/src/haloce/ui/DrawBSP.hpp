#pragma once

#include "haloce/Halo1.hpp"

namespace HaloCE::Mod::UI  {
    void drawBSPPoints(Halo1::CollisionBSP* bsp, Vec3 origin, Vec3 x, Vec3 y, Vec3 z);
    void drawBSP(Halo1::CollisionBSP* bsp, Vec3 origin, Vec3 x, Vec3 y, Vec3 z);
}
