#pragma once

#include "math/Vectors.hpp"

namespace HaloCE::Freecam {
    void init(uintptr_t halo1);
    void free();
    void update();

    struct Override {
        bool enablePosition = false;
        bool enableLook = false;
        Vec3 position;
        Vec3 forward;
        Vec3 up;
    };

    extern Override cameraOverride;
}
