#include "../MarioState.hpp"

namespace Mod::Mario {

    void moveElevatorSurfaceObject(
        uint32_t surfaceObjectId,
        const SM64ObjectTransform& transform,
        SM64MarioState& marioState)
    {
        // Query floor at Mario's XZ before and after the surface moves.
        // SM64's floor detection searches downward from Mario's Y, so a floor
        // that rises above Mario in one tick is never detected and Mario phases
        // through. Searching from well above Mario captures the new height even
        // when it has risen past him.
        float mx      = marioState.position[0];
        float mz      = marioState.position[2];
        float searchY = marioState.position[1] + 200.0f;

        SM64SurfaceCollisionData* floorDataBefore = nullptr;
        float floorBefore = sm64_surface_find_floor(mx, searchY, mz, &floorDataBefore);

        sm64_surface_object_move(surfaceObjectId, &transform);

        SM64SurfaceCollisionData* floorDataAfter = nullptr;
        float floorAfter = sm64_surface_find_floor(mx, searchY, mz, &floorDataAfter);

        // If the floor rose, apply the same delta to Mario so he rides the elevator
        // rather than being snapped to the exact floor height.
        if (floorAfter > floorBefore) {
            float delta = floorAfter - floorBefore;
            float marioY = marioState.position[1];
            float newY = marioY < floorAfter ? floorAfter : marioY + delta;
            sm64_set_mario_position(marioId, mx, newY, mz);
            marioState.position[1] = newY; // keep local Y in sync for subsequent bones
        }
    }

}
