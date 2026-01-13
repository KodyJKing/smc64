// Reverse engineered Halo 1 structures and access functions go here.

#pragma once

#include "common.hpp"
#include "math.hpp"
#include "tag.hpp"
#include "entity/index.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "map.hpp"
#include "bsp/index.hpp"
#include "tags/index.hpp"

namespace Halo1 {
    WeaponProjectileData * getProjectileData(Tag * tag, uint32_t projectileIndex);
    bool isCameraLoaded();
    bool isGameLoaded();
}
