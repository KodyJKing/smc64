#include "MarioInput.hpp"
#include <Xinput.h>
#pragma comment( lib, "Xinput.lib" )

#include "haloce/halo1/halo1.hpp"
#include "../FreeCam.hpp"
#include "Coordinates.hpp"

namespace HaloCE::Mod::Mario {

    void updateFollowCamera(Halo1::Camera* camera, SM64MarioInputs& inputs, SM64MarioState& marioState) {        
        inputs.camLookX = -camera->fwd.x;
        inputs.camLookZ = -camera->fwd.y;

        Vec3 cameraRight = camera->fwd.cross( camera->up );
        
        Freecam::cameraOverride.enablePosition = true;
        // Position the camera behind Mario
        Vec3 marioHaloPos = Coordinates::marioToHalo(Vec3{
            marioState.position[0],
            marioState.position[1],
            marioState.position[2]
        });

        Freecam::cameraOverride.position = marioHaloPos - camera->fwd * 3.0f + cameraRight * 0.25f + Vec3{0, 0, 0.5f};
    }

    
    void updateXboxControls(SM64MarioInputs& inputs) {

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

        // Buttons
        inputs.buttonA |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 1 : 0;
        inputs.buttonB |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 1 : 0;
        inputs.buttonZ |= (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1 : 0;
    }

    void updateKeyboardControls(SM64MarioInputs& inputs) {
        static uint64_t stickActiveUntil = 0;
        uint64_t now = GetTickCount64();
        
        // WASD for movement
        float x = 0.0f;
        float y = 0.0f;
        if (GetAsyncKeyState('A') & 0x8000) x -= 1.0f;
        if (GetAsyncKeyState('D') & 0x8000) x += 1.0f;
        if (GetAsyncKeyState('W') & 0x8000) y += 1.0f;
        if (GetAsyncKeyState('S') & 0x8000) y -= 1.0f;
        if (x != 0.0f || y != 0.0f) {
            inputs.stickX = x;
            inputs.stickY = y;
            stickActiveUntil = now + 100;
        } else if (now < stickActiveUntil) {
            inputs.stickX = 0;
            inputs.stickY = 0;
        }

        // Buttons
        inputs.buttonA |= (GetAsyncKeyState(VK_SPACE) & 0x8000) ? 1 : 0;
        inputs.buttonB |= (GetAsyncKeyState('F') & 0x8000) ? 1 : 0;
        inputs.buttonZ |= (GetAsyncKeyState(VK_SHIFT) & 0x8000) ? 1 : 0;
    }

    void updateInput(SM64MarioInputs& inputs, SM64MarioState& marioState, Halo1::Camera* camera) {
        inputs = {};
        updateXboxControls(inputs);
        updateKeyboardControls(inputs);
        if (camera) {
            updateFollowCamera(camera, inputs, marioState);
        }
    }

}
