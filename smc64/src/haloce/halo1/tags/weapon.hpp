#pragma once

#include <stdint.h>
#include "../tag.hpp"

namespace Halo1 {

    struct WeaponTagData {
        char pad_0000[0x4FC];
        BlockPointer projectileData;
    }; // This size has not been confirmed
    static_assert(sizeof(WeaponTagData) == 0x508, "Halo1::WeaponTagData has incorrect size.");

    struct WeaponProjectileData {
        char pad_0000[0x4];
        float minRateOfFire;
        float maxRateOfFire;
        char pad_000C[0x108];
    }; // This size has not been confirmed
    static_assert(sizeof(WeaponProjectileData) == 0x114, "Halo1::WeaponProjectileData has incorrect size.");

}
