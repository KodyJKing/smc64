// Reverse engineered Halo 1 structures and access functions go here.

#pragma once

#include <stdint.h>
#include <functional>
#include <string>
#include <vector>
#include "math/Vectors.hpp"

#include "memory/Memory.hpp"

#include "common.hpp"
#include "math.hpp"
#include "tag.hpp"
#include "entity/index.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "map.hpp"
#include "bsp/index.hpp"
#include "tags/index.hpp"

#define NULL_HANDLE 0xFFFFFFFF

namespace Halo1 {

    #pragma pack(push, 1)
        struct WeaponTagData {
            char pad_0000[0x4FC];
            BlockPointer projectileData;
        };

        struct WeaponProjectileData {
            char pad_0000[0x4];
            float minRateOfFire;
            float maxRateOfFire;
            char pad_000C[0x108];
        }; // Size = 0x114
    #pragma pack(pop)

    // Created with ReClass.NET 1.2 by KN4CK3R
    class ProjectileTagData
    {
        public:
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
    }; //Size: 0x01F0

    WeaponProjectileData * getProjectileData(Tag * tag, uint32_t projectileIndex);

    bool isCameraLoaded();
    bool isGameLoaded();

}
