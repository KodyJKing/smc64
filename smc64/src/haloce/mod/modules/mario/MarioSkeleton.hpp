#pragma once

#include <cstdint>
#include <vector>
#include "libsm64.h"
#include "math/Vectors.hpp"
#include "haloce/halo1/halo1.hpp"

namespace HaloCE::Mod::Mario {
    extern std::vector<Halo1::WorldTransform> marioPose;

    void updateMarioPose(SM64MarioGeometryBuffers &marioGeometry);
    void drawMarioBones(SM64MarioGeometryBuffers& marioGeometry);
    void dumpSkeleton(SM64MarioGeometryBuffers &marioGeometry, Vec3 marioPos, FILE *file);
    Halo1::WorldTransform getMarioBoneByName(const char *name);
    Halo1::WorldTransform* getMarioBonePointerByName(const char* name);
}
