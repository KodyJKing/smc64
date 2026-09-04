#include "PowderKeg.hpp"
#include "../functions/MarioFunctions.hpp"
#include "../functions/KillPlayer.hpp"
#include "../MarioSkeleton.hpp"
#include "../MarioState.hpp"
#include "engine/halo1.hpp"
#include "spark/input/Bindings.hpp"
#include "decomp/sm64.h"
#include "decomp/audio_defines.h"
#include "../MarioGameSpeed.hpp"

namespace Mod::Mario::PowderKeg {

    bool kegEquipped = false;

    bool isKegEquiped() {
        return kegEquipped;
    }

    void setKegEquiped(bool active) {
        kegEquipped = active;
    }

    void updatePose() {
        // Copy chest transform to powder_keg
        auto chest = getMarioBonePointerByName("frame chest");
        auto keg = getMarioBonePointerByName("frame powder_keg");
        if (chest && keg) {
            *keg = *chest;
            keg->pos += keg->y * -0.08f;
            keg->pos += keg->x * 0.02f;
        }
    }

    void spawnExplosionEffect() {
        auto playerHandle = Engine::getPlayerHandle();
        if (playerHandle == NULL_HANDLE) return;
        uint32_t effectTagHandle = Engine::findTag("smc64\\powder_keg\\explosion", "effe")->tagID;
        Engine::effectNewOnObjectMarker(effectTagHandle, playerHandle, "head");
        uint32_t smokeEffectTagHandle = Engine::findTag("smc64\\powder_keg\\smoke", "effe")->tagID;
        Engine::effectNewOnObjectMarker(smokeEffectTagHandle, playerHandle, "head");
    }

    void dealBoostDamage(float amount = 1.0f) {
        Mod::Mario::damagePlayer(amount, 1.0f);
        grantFreeBulletTime(1000);
    }

    void faceLook() {
        auto camera = Engine::getPlayerCameraPointer();
        if (!camera) return;
        float yaw = atan2f(camera->fwd.x, camera->fwd.y);
        sm64_set_mario_faceangle(marioId, yaw);
    }

    void superDive(bool setFaceAngle) {
        auto camera = Engine::getPlayerCameraPointer();
        Vec3 velocity = camera->fwd * 0.25f;
        launchMario(velocity, setFaceAngle);
        dealBoostDamage();
        spawnExplosionEffect();
        sm64_play_sound_global(SOUND_OBJ_CANNON4);
    }

    void superWallKick() {
        sm64_set_mario_forward_velocity(marioId, 100.0f);
        sm64_play_sound_global(SOUND_OBJ_CANNON4);
    }

    void onActionStart(uint32_t action, uint32_t prevAction) {
        bool wasTripleJump = (prevAction == ACT_TRIPLE_JUMP);
        switch (action) {
            case ACT_DIVE:
                superDive(wasTripleJump);
                break;
            case ACT_PUNCHING:
            case ACT_MOVE_PUNCHING:
            case ACT_JUMP_KICK:
            case ACT_SLIDE_KICK:
                faceLook();
                dealBoostDamage();
            // case ACT_WALL_KICK_AIR:
                sm64_set_mario_forward_velocity(marioId, 100.0f);
                sm64_play_sound_global(SOUND_OBJ_CANNON4);
                spawnExplosionEffect();
                break;
            case ACT_GROUND_POUND_LAND:
                sm64_play_sound_global(SOUND_OBJ_CANNON4);
                sm64_set_mario_action(marioId, ACT_LAVA_BOOST);
                sm64_set_mario_forward_velocity(marioId, 50.0f);
                sm64_set_mario_velocity(marioId, 0, 100.0f, 0);
                dealBoostDamage();
                spawnExplosionEffect();
                break;
            default:
                break;
        }
    }

    void updateMarioBonkDamage() {
        auto v0 = marioState.velocity[0];
        auto v1 = marioState.velocity[1];
        auto v2 = marioState.velocity[2];
        auto currentAction = marioState.action;
        auto currentSpeed = sqrtf(v0 * v0 + v1 * v1 + v2 * v2);
        static auto lastAction = currentAction;
        static float lastSpeed = currentSpeed;

        auto bonkDamage = [](uint32_t action) {
            if (action == ACT_GROUND_BONK) return 1;
            if (action == ACT_BACKWARD_GROUND_KB) return 1;
            if (action == ACT_BACKWARD_AIR_KB) return 1;
            if (action == ACT_SOFT_BONK) return 1;
            return 0;
        };

        if (bonkDamage(currentAction) && !bonkDamage(lastAction)) {
            auto damage = bonkDamage(currentAction) * lastSpeed / 16.0f;
            Mod::Mario::damagePlayer((float) damage, 1.0f);
            if (damage > 5.0f) spawnExplosionEffect();
        }

        lastAction = currentAction;
        lastSpeed = currentSpeed;
    }
    
    void updateControls() {
        auto currentAction = marioState.action;
        auto static lastAction = currentAction;

        // auto animFrame = marioState.animFrame;
        // if (animFrame == 0 && currentAction != lastAction) {
        if (currentAction != lastAction) {
            onActionStart(currentAction, lastAction);
        }

        lastAction = currentAction;
    }

    void update() {
        if (!isKegEquiped()) return;
        updatePose();
        updateControls();
        updateMarioBonkDamage();
    }

}
