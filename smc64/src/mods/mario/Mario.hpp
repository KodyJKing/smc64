#pragma once

#define ENABLE_MARIO 1

#include "MarioState.hpp"
#include "MarioModel.hpp"
#include "functions/MarioWorldPosition.hpp"
#include "math/Vectors.hpp"
#include "spark/mod/ModId.hpp"

namespace Mod::Mario {

    // Check that the user has the required SM64 ROM file installed.
    bool checkRom(bool showErrorMessage = true);
    
    void init(Spark::ModId modId);
    void free();
    void update();
    void debugRender();
    void deinitMario();

}
