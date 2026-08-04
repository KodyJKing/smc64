#pragma once

#include "libsm64.h"
#include "math/Vectors.hpp"
#include <cstdint>
#include <cstddef>

namespace Mod::Mario {
    extern bool enableMario;
    extern bool possessMario;

    extern uint8_t* texture;
    extern uint8_t* rom;
    extern size_t romSize;

    extern int32_t marioId;
    extern uint64_t marioTickCount;
    extern SM64MarioInputs marioInputs;
    extern SM64MarioState marioState;
    extern SM64MarioGeometryBuffers marioGeometry;
    extern SM64Matrix4f marioBoneMatrices[SM64_MARIO_BONE_COUNT];

    // Chunk that marioState.position is expressed relative to.
    // True world mario pos = Coordinates::marioChunkOrigin(marioChunk) + marioState.position.
    extern Vec3i marioChunk;

    Vec3 getMarioPosition();
    void setMarioPosition(const Vec3& position);

    bool marioExists();
    bool marioDead();
    bool marioInControl();
    bool marioAirborne();
    bool marioRidingShell();
    bool marioCanCollideWithBipeds();
}
