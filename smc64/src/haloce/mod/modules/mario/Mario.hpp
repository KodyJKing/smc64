#pragma once

#include "libsm64.h"
#include "MarioModel.hpp"

namespace HaloCE::Mod::Mario {
    extern SM64MarioState marioState;
    void init();
    void free();
    void update();
    void debugRender();
}
