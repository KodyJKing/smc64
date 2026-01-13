#pragma once

#include <stdint.h>
#include "../tag.hpp"

namespace Halo1 {
    struct ProjectileTagData {
        char pad_0000[444]; //0x0000
        float minExplodeTime; //0x01BC
        float maxExplodeTime; //0x01C0
        char pad_01C4[4]; //0x01C4
        float lifeSpan; //0x01C8
        float arc; //0x01CC
        char pad_01D0[20]; //0x01D0
        float initialSpeed; //0x01E4
        float finalSpeed; //0x01E8
        float homing; //0x01EC
    };
    static_assert(sizeof(ProjectileTagData) == 0x1F0, "Halo1::ProjectileTagData has incorrect size.");
}
