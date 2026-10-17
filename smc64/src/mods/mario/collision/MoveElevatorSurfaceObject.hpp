#pragma once

#include "libsm64.h"

namespace Mod::Mario {
    // Moves a SM64 surface object to the given transform and, if the floor at
    // Mario's XZ rose past him as a result, pushes Mario up to ride it.
    // marioState.position[1] is updated in-place so callers iterating over
    // multiple bones see a consistent Mario Y for each subsequent query.
    void moveElevatorSurfaceObject(
        uint32_t surfaceObjectId,
        const SM64ObjectTransform& transform,
        SM64MarioState& marioState);
}
