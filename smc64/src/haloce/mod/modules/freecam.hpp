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

        Vec3 (*getPosition)() = nullptr;
        Vec3 (*getForward)() = nullptr;
        Vec3 (*getUp)() = nullptr;
    };

    extern Override cameraOverride;
}
