#pragma once

#include <stdint.h>
#include "math/Vectors.hpp"

namespace Halo1 {

    struct ProjectileSpawnArgs {
        uint32_t projectileTagId;
        uint32_t unknown1;
        uint32_t unknown2;
        uint32_t unknown3;
        uint32_t ownerEntityHandle;
        uint32_t unknown4;
        uint32_t unknown5;
        Vec3 spawnPosition;
    };

}
