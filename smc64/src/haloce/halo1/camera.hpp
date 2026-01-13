#pragma once
#include "math/Vectors.hpp"

namespace Halo1 {
    #pragma pack(push, 1)
    struct Camera {
        char pad0[4];
        Vec3 pos;
        char pad1[16];
        float fov;
        Vec3 fwd;
        Vec3 up;
    };
    #pragma pack(pop)
}
