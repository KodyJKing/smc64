#include "MarioInput.hpp"
#include <Xinput.h>
#pragma comment( lib, "Xinput.lib" )

#include "haloce/halo1/halo1.hpp"
#include "../FreeCam.hpp"
#include "Coordinates.hpp"

namespace HaloCE::Mod::Mario {

    
    void updateXboxControls(SM64MarioInputs& inputs, SM64MarioState& marioState) {

        static uint64_t stickActiveUntil = 0;
        uint64_t now = GetTickCount64();

        XINPUT_STATE state = {0};
        XInputGetState(0, &state);

        // Left stick for movement.
        float x = state.Gamepad.sThumbLX / 32768.0f;
        float y = state.Gamepad.sThumbLY / 32768.0f;
        float r = sqrtf(x * x + y * y);
        float deadzone = XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE / 32768.0f;
        if (r > deadzone) {
            inputs.stickX = state.Gamepad.sThumbLX / 32768.0f;
            inputs.stickY = state.Gamepad.sThumbLY / 32768.0f;
            stickActiveUntil = now + 100;
        } else if (now < stickActiveUntil) {
            inputs.stickX = 0;
            inputs.stickY = 0;
        }

        auto playerCam = Halo1::getPlayerCameraPointer();
        if (playerCam) {
            inputs.camLookX = -playerCam->fwd.x;
            inputs.camLookZ = -playerCam->fwd.y;

            Freecam::cameraOverride.enablePosition = true;
            Vec3 marioHaloPos = Coordinates::marioToHalo(Vec3{
                marioState.position[0],
                marioState.position[1],
                marioState.position[2]
            });
            Freecam::cameraOverride.position = marioHaloPos - playerCam->fwd * 3.0f + Vec3{0, 0, 0.5f};
        }

        // Buttons
        inputs.buttonA = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 1 : 0;
        inputs.buttonB = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 1 : 0;
        inputs.buttonZ = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1 : 0;
    }

}
