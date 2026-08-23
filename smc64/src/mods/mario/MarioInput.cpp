#include "MarioInput.hpp"
#include "spark/input/Bindings.hpp"
#define DIRECTINPUT_VERSION 0x0800
#include <dinput.h>

namespace Mod::Mario {

    void registerInputActions() {
        Spark::Input::addAction("mario:bullet_time", { SPARK_GAMEPAD_LEFT_THUMB, SPARK_GAMEPAD_RIGHT_SHOULDER, DIK_LCONTROL });
        
        Spark::Input::addAction("mario:crouch", { SPARK_GAMEPAD_LEFT_SHOULDER, DIK_LSHIFT });
        Spark::Input::addAction("mario:jump",       {SPARK_GAMEPAD_A, DIK_SPACE });
        Spark::Input::addAction("mario:punch",      {SPARK_GAMEPAD_B, DIK_F});
        
        Spark::Input::addAction("mario:walk_right", {SPARK_GAMEPAD_LEFT_STICK_RIGHT, DIK_D});
        Spark::Input::addAction("mario:walk_left",  {SPARK_GAMEPAD_LEFT_STICK_LEFT, DIK_A});
        Spark::Input::addAction("mario:walk_fwd",   {SPARK_GAMEPAD_LEFT_STICK_UP, DIK_W});
        Spark::Input::addAction("mario:walk_back",  {SPARK_GAMEPAD_LEFT_STICK_DOWN, DIK_S});

        Spark::Input::addAction("mario:camera_zoom_in", { SPARK_GAMEPAD_DPAD_UP, DIK_EQUALS });
        Spark::Input::addAction("mario:camera_zoom_out", { SPARK_GAMEPAD_DPAD_DOWN, DIK_MINUS });
        Spark::Input::addAction("mario:camera_zoom_toggle", { SPARK_GAMEPAD_LEFT_THUMB, DIK_0 });
    }

    void updateInput(SM64MarioInputs& inputs, SM64MarioState& marioState, Engine::Camera* camera) {
        inputs = {};
        inputs.buttonZ = (Spark::Input::actionState("mario:crouch") & 0x80) ? 1 : 0;
        inputs.buttonA = (Spark::Input::actionState("mario:jump")   & 0x80) ? 1 : 0;
        inputs.buttonB = (Spark::Input::actionState("mario:punch")  & 0x80) ? 1 : 0;

        inputs.stickX  = Spark::Input::actionAxis("mario:walk_right") - Spark::Input::actionAxis("mario:walk_left");
        inputs.stickY  = Spark::Input::actionAxis("mario:walk_fwd")   - Spark::Input::actionAxis("mario:walk_back");

        const float deadZone = 0.05f;
        if (fabs(inputs.stickX) < deadZone) inputs.stickX = 0.0f;
        if (fabs(inputs.stickY) < deadZone) inputs.stickY = 0.0f;

        float length = sqrt(inputs.stickX * inputs.stickX + inputs.stickY * inputs.stickY);
        if (length > 1.0f) {
            inputs.stickX /= length;
            inputs.stickY /= length;
        }

        if (camera) {
            inputs.camLookX = -camera->fwd.x;
            inputs.camLookZ = -camera->fwd.y;
        }
    }

}

