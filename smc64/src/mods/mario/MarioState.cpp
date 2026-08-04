#include "MarioState.hpp"
#include "math/Vectors.hpp"
#include "engine/halo1.hpp"
#include "decomp/sm64.h"
#include "level-edit/MarioLevelEdit.hpp"
#include "MarioBSPChunk.hpp"

namespace Mod::Mario {
    bool enableMario = true;
    bool possessMario = true;

    uint8_t* texture = nullptr;
    uint8_t* rom;
    size_t romSize = 0;

    int32_t marioId = -1;
    uint64_t marioTickCount = 0;
    SM64MarioInputs marioInputs;
    SM64MarioState marioState;
    SM64MarioGeometryBuffers marioGeometry;
    SM64Matrix4f marioBoneMatrices[SM64_MARIO_BONE_COUNT] = {};

    Vec3i marioChunk = { 0, 0, 0 };

    // Mario's position in Mario coordinates.
    Vec3 getMarioPosition() {
        return {
            marioState.position[0],
            marioState.position[1],
            marioState.position[2]
        };
    }

    void setMarioPosition(const Vec3& position) {
        sm64_set_mario_position(marioId, position.x, position.y, position.z);
    }

    bool marioExists() {
        return marioId >= 0;
    }

    bool marioDead() {
        auto playerEntity = Engine::getPlayerEntity();
        if (!playerEntity) return true;
        return playerEntity->health <= 0;
    }

    bool marioInControl() {
        auto playerEntity = Engine::getPlayerEntity();
        if (!playerEntity) return false;

        if (playerEntity->vehicleHandle != NULL_HANDLE) return false;
        if (playerEntity->parentHandle != NULL_HANDLE) return false;

        if (playerEntity->health <= 0) return false;

        if (marioId < 0) return false;

        if (Engine::isPlayerInputDisabled()) return false;

        return enableMario && possessMario && !::Mod::Mario::LevelEdit::isInputSuppressed() && MarioBSPChunk::hasValidLoadedChunk();
    }

    bool marioAirborne() {
        return (marioState.action & ACT_FLAG_AIR) != 0;
    }

    bool marioRidingShell() {
        return (marioState.action & ACT_FLAG_RIDING_SHELL) != 0;
    }

    bool marioAttacking() {
        return (marioState.action & ACT_FLAG_ATTACKING) != 0;
    }

    bool marioCanCollideWithBipeds() {
        return !marioRidingShell() && !marioAttacking() && !marioAirborne();
    }
}
