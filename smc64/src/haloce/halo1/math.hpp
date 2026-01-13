#pragma once

#include "math/Vectors.hpp"

namespace Halo1 {
    // Used for relative bone transforms.
    struct Transform {
        Quaternion rotation;
        Vec3 translation;
        float scale;
    };

    // Used for world sppace bone transforms.
    struct WorldTransform {
        float w; // (scale)
        Vec3 x, y, z, pos; // (forward, left, up, translation)
    };
}
