#pragma once

#include <cstdint>
#include "libsm64.h"
#include "math/Vectors.hpp"

namespace HaloCE::Mod::Mario {
    void drawMarioBones(SM64MarioGeometryBuffers& marioGeometry);
    void dumpSkeleton(SM64MarioGeometryBuffers &marioGeometry, Vec3 marioPos, FILE *file);
}
