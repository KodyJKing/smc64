#include "MarioInput.hpp"
#include <Xinput.h>
#pragma comment( lib, "Xinput.lib" )

#include "engine/halo1.hpp"
#include "Coordinates.hpp"

namespace Mod::Mario {
    
    void updateXboxControls(SM64MarioInputs& inputs) {
        XINPUT_STATE state = {0};
        XInputGetState(0, &state);

        inputs.buttonZ |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1 : 0;
    }

    void updateKeyboardControls(SM64MarioInputs& inputs) {
        // Buttons
        inputs.buttonZ |= (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 1 : 0;
    }

    void updateMappedHaloControls(SM64MarioInputs& inputs) {
        auto playerController = Engine::getPlayerControllerPointer();
        if (playerController) {
            bool cheifMelee = (playerController->actions & Engine::PlayerActionFlags::melee) != 0;
            bool cheifJump = (playerController->actions & Engine::PlayerActionFlags::jump) != 0;
            if (cheifMelee) inputs.buttonB = 1;
            if (cheifJump) inputs.buttonA = 1;
            inputs.stickX = -playerController->walkX;
            inputs.stickY = playerController->walkY;
        }
    }

    void updateInput(SM64MarioInputs& inputs, SM64MarioState& marioState, Engine::Camera* camera) {
        inputs = {};
        updateXboxControls(inputs);
        updateKeyboardControls(inputs);
        updateMappedHaloControls(inputs);
        
        if (camera) {
            inputs.camLookX = -camera->fwd.x;
            inputs.camLookZ = -camera->fwd.y;
        }
    }

}
