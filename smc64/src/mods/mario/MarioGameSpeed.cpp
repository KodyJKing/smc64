#include "MarioGameSpeed.hpp"

#include "MarioAudio.hpp"
#include "MarioState.hpp"
#include "engine/halo1.hpp"
#include "spark/input/Bindings.hpp"

#include <cstdio>

#include "decomp/sm64.h"

namespace Mod::Mario {

    float smoothedGameSpeed = 1.0f;
    float gamespeed = 1.0f;

    uint64_t bulletTimeFreeUntil = 0;

    float shieldCostMultiplier() {
        uint64_t currentTick = GetTickCount64();
        if (currentTick < bulletTimeFreeUntil) {
            return 0.0f; // No shield cost during free bullet time period.
        }

        if (marioState.action == ACT_CRAZY_BOX_BOUNCE) {
            return 0.0f;
        }
        
        return 1.0f;
    }

    void setGameSpeed(float speed) {
        // if (gamespeed == speed) return;
        gamespeed = speed;
        char buffer[64];
        snprintf(buffer, sizeof(buffer), "game_speed_value %.2f", speed);
        Engine::Scripting::submit(buffer);
        MarioAudio::setGameSpeed(speed);
    }

    bool bulletTimeButtonDown() {
        return Spark::Input::actionState("mario:bullet_time") & 0x80;
    }

    void updateGameSpeed(Engine::Entity& player) {
        float costMultiplier = shieldCostMultiplier();
        bool airborne = marioAirborne();
        bool hasSheilds = player.shield > 0 || costMultiplier == 0.0f;
        bool canSlowdown = airborne && hasSheilds;
        bool slowDown = canSlowdown && bulletTimeButtonDown();
        if (slowDown) {
            player.shield -= 0.05f * costMultiplier;
            gamespeed = 0.25f;
        } else {
            gamespeed = 1.0f;
        }

        // Smoothly interpolate the game speed to avoid abrupt changes.
        float smoothingFactor = 0.4f; // Adjust this for faster/slower smoothing
        smoothedGameSpeed += (gamespeed - smoothedGameSpeed) * smoothingFactor;

        setGameSpeed(smoothedGameSpeed);
    }

    void grantFreeBulletTime(uint64_t durationMillis) {
        uint64_t currentTick = GetTickCount64();
        bulletTimeFreeUntil = currentTick + durationMillis;
    }

}
