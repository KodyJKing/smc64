#pragma once
#include "libsm64.h"
#include "engine/halo1.hpp"

namespace Mod::Mario {
    void registerInputActions();
    void updateInput(SM64MarioInputs& inputs, SM64MarioState& marioState, Engine::Camera* camera = nullptr);
}
