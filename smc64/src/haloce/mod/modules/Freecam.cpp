#include "freecam.hpp"
#include "common.hpp"
#include <Xinput.h>
#pragma comment( lib, "Xinput.lib" )

#define HOOK_FUNC( func, offset) \
    void* p##func = (void*) (halo1 + offset); \
    std::cout << #func << ": " << std::endl; \
    std::cout << AsmHelper::disassemble( (uint8_t*) p##func, 0x100 ) << std::endl; \
    MH_CreateHook( p##func, hk##func, (void**) &original##func ); \
    MH_EnableHook( p##func );

namespace HaloCE::Freecam {

    bool isFreecamEnabled = false;

    Override cameraOverride = {};

    bool isEnabled() {
        return isFreecamEnabled || cameraOverride.enableLook || cameraOverride.enablePosition;
    }

    Vec3 getCameraPosition(Halo1::Camera* camera) {
        if (!cameraOverride.enablePosition)
            return camera->pos;
        if (cameraOverride.getPosition)
            return cameraOverride.getPosition();
        return cameraOverride.position;
    }

    // void renderFPVModel(void)
    typedef void (*renderFPVModel)();
    renderFPVModel originalRenderFPVModel = nullptr;
    void hkRenderFPVModel() {
        UnloadLock lock; // No unloading while we're still executing hook code.
        if (isEnabled()) {
            return;
        }
        originalRenderFPVModel();
    }

    // void updatePlayerControls(undefined4 *param_1,undefined4 *param_2)
    typedef void (*updatePlayerControls)(float* param_1, float* param_2);
    updatePlayerControls originalUpdatePlayerControls = nullptr;
    void hkUpdatePlayerControls(float* param_1, float* param_2) {
        UnloadLock lock; // No unloading while we're still executing hook code.
        if (!isEnabled()) {
            originalUpdatePlayerControls(param_1, param_2);
            return;
        }

        auto playerController = Halo1::getPlayerControllerPointer();
        if (!playerController || !Memory::isAllocated(playerController)) {
            return;
        }

        // Backup player controller state
        Halo1::PlayerController pc = *playerController;

        // playerController->actions = 0;
        playerController->walkX = 0.0f;
        playerController->walkY = 0.0f;
        // playerController->gunTrigger = 0.0f;

        originalUpdatePlayerControls(param_1, param_2);

        // Restore player controller state
        *playerController = pc;

        return;
    }

    // Slight misnomer: This function updates *all* cameras, not a single camera.
    typedef void (*updateCamera)(float unknown);
    updateCamera originalUpdateCamera = nullptr;
    void hkUpdateCamera(float unknown) {
        UnloadLock lock; // No unloading while we're still executing hook code.

        if (!isEnabled()) {
            originalUpdateCamera(unknown);
            return;
        }
        
        auto camera = Halo1::getPlayerCameraPointer();
        bool camAllocated = camera && Memory::isAllocated(camera);
        Vec3 camPos = {0,0,0};
        bool saveCamPos = camAllocated && isFreecamEnabled;

        // Halo1::enterThirdPerson();

        if (saveCamPos) {
            camPos = camera->pos;
        }
        originalUpdateCamera(unknown);
        if (saveCamPos) {
            camera->pos = camPos;
        } else {
            camera->pos = getCameraPosition(camera);
        }

        // Halo1::enterThirdPerson();
    }

    void init(uintptr_t halo1) {
        std::cout << "\nHooking Freecam functions:\n" << std::endl;

        HOOK_FUNC( UpdateCamera, 0xB14380U );
        HOOK_FUNC( UpdatePlayerControls, 0xA9A8A4U );
        HOOK_FUNC( RenderFPVModel, 0xB275B8U );
    }
    
    void free() {
        std::cout << "\nUnhooking Freecam functions." << std::endl;
        MH_RemoveHook( (void*) originalUpdateCamera );
        MH_RemoveHook( (void*) originalUpdatePlayerControls );
        MH_RemoveHook( (void*) originalRenderFPVModel );
    }

    void updateXboxControls(Halo1::Camera* camera) {
        XINPUT_STATE state;
        ZeroMemory(&state, sizeof(XINPUT_STATE));
        XInputGetState(0, &state);

        float speed = 0.2f;
        if (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER)
            speed *= 5.0f; // Fast
        if (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER)
            speed *= 0.2f; // Slow
        
        Vec3 fwd = camera->fwd;
        Vec3 up = camera->up;
        Vec3 right = fwd.cross( up );

        Vec3 moveDelta = {};

        // Left stick for horizontal movement.
        moveDelta += fwd * ( state.Gamepad.sThumbLY / 32768.0f * speed );
        moveDelta += right * ( state.Gamepad.sThumbLX / 32768.0f * speed );

        // L/R triggers for vertical movement.
        moveDelta += up * ( (state.Gamepad.bRightTrigger / 255.0f) * speed );
        moveDelta -= up * ( (state.Gamepad.bLeftTrigger / 255.0f) * speed );
    
        camera->pos += moveDelta;
    }

    void updateKeyboardControls(Halo1::Camera* camera) {

        float speed = 0.2f;
        if (GetAsyncKeyState(VK_MENU) & 0x8000)
            speed *= 5.0f; // Fast
        if (GetAsyncKeyState(VK_CONTROL) & 0x8000)
            speed *= 0.2f; // Slow
        
        Vec3 fwd = camera->fwd;
        Vec3 up = camera->up;
        Vec3 right = fwd.cross( up );

        // WASD for horizontal movement.
        if (GetAsyncKeyState('W') & 0x8000)
            camera->pos += fwd * speed;
        if (GetAsyncKeyState('S') & 0x8000)
            camera->pos -= fwd * speed;
        if (GetAsyncKeyState('A') & 0x8000)
            camera->pos -= right * speed;
        if (GetAsyncKeyState('D') & 0x8000)
            camera->pos += right * speed;
        
        // QE for vertical movement.
        if (GetAsyncKeyState('R') & 0x8000)
            camera->pos += up * speed;
        if (GetAsyncKeyState('F') & 0x8000)
            camera->pos -= up * speed;
    }

    void update() {
        if (GetAsyncKeyState(VK_HOME) & 1) {
            isFreecamEnabled = !isFreecamEnabled;
            std::cout << "Freecam " << (isFreecamEnabled ? "enabled." : "disabled.") << std::endl;
        }
        if (!isEnabled()) return;

        auto camera = Halo1::getPlayerCameraPointer();
        if (!camera || !Memory::isAllocated(camera)) return;

        if (isFreecamEnabled) {
            updateXboxControls(camera);
            updateKeyboardControls(camera);
        } else {
            // camera->pos = cameraOverride.position;
        }

    }
    
}
