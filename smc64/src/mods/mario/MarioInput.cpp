#include "MarioInput.hpp"
#include "spark/input/Bindings.hpp"
#include <Xinput.h>
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>
#pragma comment( lib, "Xinput.lib" )

#include "engine/halo1.hpp"
#include "Coordinates.hpp"

namespace Mod::Mario {

    void registerInputActions() {
        const Spark::Input::ButtonCode kCrouch[]     = { SPARK_GAMEPAD_LEFT_SHOULDER, DIK_LSHIFT };
        Spark::Input::addAction("mario:crouch", kCrouch, 2);
        const Spark::Input::ButtonCode kBulletTime[] = { SPARK_GAMEPAD_DPAD_UP, SPARK_GAMEPAD_LEFT_THUMB, SPARK_GAMEPAD_RIGHT_SHOULDER, DIK_LCONTROL };
        Spark::Input::addAction("mario:bullet_time", kBulletTime, 4);
    }

    void updateXboxControls(SM64MarioInputs& inputs) {
        XINPUT_STATE state = {0};
        XInputGetState(0, &state);

        inputs.buttonZ |= (Spark::Input::actionState("mario:crouch") & 0x80) ? 1 : 0;
    }

    void updateKeyboardControls(SM64MarioInputs& inputs) {
        // Buttons
        inputs.buttonZ |= (Spark::Input::actionState("mario:crouch") & 0x80) ? 1 : 0;
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
