#include "MarioCamera.hpp"
#include "spark/hook/Hooks.hpp"
#include "spark/input/Bindings.hpp"
#include "memory/Memory.hpp"
#include "halomcc/HaloMCC.hpp"
#include "MarioState.hpp"
#include <iostream>
#include "functions/CheifToMario.hpp"
#include "decomp/sm64.h"
#include "decomp/audio_defines.h"
#include "libsm64.h"

// #define DEBUG_MARIO_CAMERA 1
//
#ifdef DEBUG_MARIO_CAMERA
#include "spark/overlay/Gizmos.hpp"
#endif

namespace Mod::Mario::MarioCamera {

    static bool     active               = false;
    static float    camDistanceScale      = 1.0f;

    static constexpr float CAM_BACK        = -1.2f;
    static constexpr float CAM_RIGHT       =  0.125f;
    static constexpr float CAM_RIGHT_FIXED =  0.125f;
    static constexpr float CAM_UP          =  0.25f;
    static constexpr float CAM_WALL_MARGIN =  0.1f;
    static constexpr float CAM_RAY_START_DIST = 0.0f;
    static constexpr float CAM_EXTEND_RATE =  0.06f; // per frame; ~1 s to full extension at 60 fps

    
    static size_t zoomLevel = 2;
    static constexpr float CAM_MAX_DISTANCE_SCALE = 4.0f;
    static float CAM_ZOOM_LEVELS[] = {0.25f, 0.5f, 1.0f, 2.0f, CAM_MAX_DISTANCE_SCALE};

    static bool isHanging() {
        return (marioState.action & ACT_FLAG_HANGING) != 0;
    }

    static float cameraUp() {
        static float result = CAM_UP;
        result = isHanging() ? 0.0f : CAM_UP;
        static float resultSmoothed = result;
        resultSmoothed = resultSmoothed * 0.9f + result * 0.1f;
        return resultSmoothed;
    }

    float getDesiredDistanceScale() {
        return CAM_ZOOM_LEVELS[zoomLevel];
    }

    static Vec3 getCameraPosition() {
        auto camera = Engine::getPlayerCameraPointer();
        if (!camera) return Vec3{0, 0, 0};

        Vec3 marioPos = camera->pos - Vec3{0,0,0.5f};
        Vec3 right    = camera->fwd.cross(camera->up);

        // Cast a ray from Mario's head height toward the desired camera position.
        // This lets us detect walls and pull the camera in to prevent clipping.
        Vec3 rayDisp   = (camera->fwd * CAM_BACK + right * CAM_RIGHT) * CAM_MAX_DISTANCE_SCALE;
        Vec3 rayOrigin = marioPos + Vec3{0, 0, cameraUp()} + rayDisp.normalize() * CAM_RAY_START_DIST;
        float rayLen   = rayDisp.length();

        float targetScale = getDesiredDistanceScale();
        if (rayLen > 0.0f) {
            Engine::RaycastResult rr;
            Engine::raycast(ENGINE_RAYCAST_PROJECTILE_FLAGS, &rayOrigin, &rayDisp,
                            Engine::getPlayerHandle(), &rr);
            if (rr.hitType != Engine::HitType_Nothing) {
                Vec3 rayDir  = rayDisp * (1.0f / rayLen);
                float hitDist = (rr.pos - rayOrigin).dot(rayDir);
                float t = (hitDist - CAM_WALL_MARGIN) / rayLen * CAM_MAX_DISTANCE_SCALE;
                if (t < 0.0f) t = 0.0f;
                if (t < targetScale) targetScale = t;
            }
        }

        // Retract immediately; extend gradually so the camera doesn't pop back through walls.
        if (targetScale < camDistanceScale)
            camDistanceScale = targetScale;
        else if (camDistanceScale < targetScale)
            camDistanceScale = camDistanceScale + CAM_EXTEND_RATE < targetScale
                             ? camDistanceScale + CAM_EXTEND_RATE
                             : targetScale;

        Vec3 result = marioPos
            + (camera->fwd * CAM_BACK + right * CAM_RIGHT) * camDistanceScale 
            + right * CAM_RIGHT_FIXED
            + Vec3{0, 0, cameraUp()};
        return result;
    }

    void updateControls() {
        static unsigned char zoomInWasPressed = 0;
        static unsigned char zoomOutWasPressed = 0;

        if (Spark::Input::actionPressed("mario:camera_zoom_out", &zoomInWasPressed)) {
            if (zoomLevel < sizeof(CAM_ZOOM_LEVELS)/sizeof(CAM_ZOOM_LEVELS[0]) - 1) {
                zoomLevel++;
                sm64_play_sound_global(SOUND_MENU_CAMERA_ZOOM_OUT);
            }
        }
        if (Spark::Input::actionPressed("mario:camera_zoom_in", &zoomOutWasPressed)) {
            if (zoomLevel > 0) {
                zoomLevel--;
                sm64_play_sound_global(SOUND_MENU_CAMERA_ZOOM_IN);
            }
        }
    }

    void onUpdate(Vec3 marioWorldPos) {
        active = true;
        updateControls();
    }

    void onDisable() {
        active = false;
        camDistanceScale = 1.0f;
    }

    void registerHandlers(Spark::ModId modId) {
        Spark::RenderFPVModel::addHandler(modId, +[](void* /*ctx*/, auto next) {
            if (active) return;
            next();
        }, nullptr, 10);

        // Suppress walk and crouch input when Mario is possessing the player.
        Spark::UpdatePlayerControls::addHandler(modId, +[](void* /*ctx*/, auto next, float* param_1, float* param_2) {
            if (!active) {
                next(param_1, param_2);
                return;
            }
            auto playerController = Engine::getPlayerControllerPointer();
            if (!playerController || !Memory::isAllocated(playerController)) return;
            Engine::PlayerController pc = *playerController;
            playerController->walkX = 0.0f;
            playerController->walkY = 0.0f;
            playerController->actions &= ~Engine::PlayerActionFlags::crouch;
            playerController->actions &= ~Engine::PlayerActionFlags::jump;
            playerController->actions &= ~Engine::PlayerActionFlags::melee;
            next(param_1, param_2);
            *playerController = pc;
        }, nullptr, 10);

        // Override camera position with Mario's interpolated position.
        Spark::UpdateCamera::addHandler(modId, +[](void* /*ctx*/, auto next, float unknown) {
            auto player = Engine::getPlayerEntity();
            if (!player || !Memory::isAllocated(player)) {
                next(unknown);
                return;
            }

            // Make sure cheif is at Mario's location before the camera update, so that the camera is positioned correctly.
            if (active && marioInControl()) {
                cheifToMario(player);
            }

            next(unknown);
            if (!active || !marioInControl()) return;
            auto camera = Engine::getPlayerCameraPointer();
            if (!camera || !Memory::isAllocated(camera)) return;

            #ifdef DEBUG_MARIO_CAMERA
            auto playerPos = player->pos;
            auto cameraPos = camera->pos;
            Spark::Overlay::Gizmos::drawLine(playerPos, cameraPos, 0xFF00FFFF, 1);
            #endif

            camera->pos = getCameraPosition();
        }, nullptr, 10);
    }

}
