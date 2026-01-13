#pragma once

#include "math/Vectors.hpp"

namespace Halo1 {
    struct Transform {
        Quaternion rotation;
        Vec3 translation;
        float scale;
    };
}