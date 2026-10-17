#include "../MarioState.hpp"
#include "MarioFalseFloor.hpp"
#include "libsm64.h"
#include "decomp/surface_terrains.h"
#include <stdint.h>

namespace Mod::Mario {

    #define W 1000.0f
    #define Y -1000.0f

    // See createSpawnPlatform for reference on how to initialize surfaces
    static SM64Surface falseFloorSurfaces[2] = {
        {
            SURFACE_DEFAULT, 0, TERRAIN_GRASS,
            {
                { -W, Y, -W },
                { -W, Y,  W },
                {  W, Y,  W }
            }
        },
        {
            SURFACE_DEFAULT, 0, TERRAIN_GRASS,
            {
                { -W, Y, -W },
                {  W, Y,  W },
                {  W, Y, -W }
            }
        }
    };
    static SM64SurfaceObject falseFloorObject = { {0}, 2, falseFloorSurfaces };
    static int64_t falseFloorObjectId = -1;

    void updateFalseFloor() {
        if (falseFloorObjectId == -1) {
            falseFloorObjectId = sm64_surface_object_create(&falseFloorObject);
        }
        falseFloorObject.transform.position[0] = marioState.position[0];
        falseFloorObject.transform.position[1] = marioState.position[1];
        falseFloorObject.transform.position[2] = marioState.position[2];
        sm64_surface_object_move(falseFloorObjectId, &falseFloorObject.transform);
    }

    void freeFalseFloor() {
        if (falseFloorObjectId != -1) {
            sm64_surface_object_delete(falseFloorObjectId);
            falseFloorObjectId = -1;
        }
    }
}
