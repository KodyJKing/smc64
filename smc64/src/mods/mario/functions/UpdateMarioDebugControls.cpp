#include "MarioFunctions.hpp"
#include "MarioToCheif.hpp"
#include "../MarioState.hpp"
#include "decomp/sm64.h"
#include "decomp/audio_defines.h"
#include "engine/halo1.hpp"
#include "spark/input/Bindings.hpp"
#include <windows.h>

namespace Mod::Mario {

    void updateMarioDebugControls() {
        if (GetAsyncKeyState(VK_F3) & 1) enableMario = !enableMario;
        if (GetAsyncKeyState(VK_F4) & 1) {
            possessMario = !possessMario;
            if (possessMario) marioToCheif();
        }

        static unsigned char debug1WasPressed = false;
        if (Spark::Input::actionPressed("mario:debug1", &debug1WasPressed)) {
            // sm64_set_mario_action_arg(marioId, ACT_RIDING_SHELL_GROUND, 0);

            // sm64_set_mario_action_arg(marioId, ACT_CRAZY_BOX_BOUNCE, 0);
            
            // sm64_set_mario_state(marioId, marioState.flags | MARIO_WING_CAP);
            // sm64_set_mario_action(marioId, ACT_FLYING_TRIPLE_JUMP);
            
            // sm64_set_mario_action(marioId, ACT_VERTICAL_WIND);
            
            // sm64_set_mario_action(marioId, ACT_HOLD_IDLE);
            
            // sm64_set_mario_velocity(marioId, 0, 150, 0);
            
            // Mod::Mario::killPlayer();
            
            // sm64_set_mario_action(marioId, ACT_GROUND_BONK);
        }
    }

}
