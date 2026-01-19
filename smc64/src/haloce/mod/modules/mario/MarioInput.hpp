#pragma once
#include "libsm64.h"
#include "haloce/halo1/halo1.hpp"

namespace HaloCE::Mod::Mario {
    void updateInput(SM64MarioInputs& inputs, SM64MarioState& marioState, Halo1::Camera* camera = nullptr);
}
