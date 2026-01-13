// Reverse engineered Halo 1 structures and access functions go here.

#include "halo1.hpp"
#include <Windows.h>
#include <iostream>
#include "memory/Memory.hpp"
#include "utils/Strings.hpp"

namespace Halo1 {

    WeaponProjectileData* getProjectileData( Tag* tag, uint32_t projectileIndex) {
        if ( !tag ) return nullptr;
        auto data = (WeaponTagData*) tag->getData();
        if ( !data ) return nullptr;
        if (data->projectileData.count <= projectileIndex) return nullptr;
        auto projectileData = (WeaponProjectileData*) translateMapAddress( data->projectileData.offset );
        return &projectileData[projectileIndex];
    }

    // =======================

    bool isCameraLoaded() { return Memory::isAllocated( (uintptr_t) getPlayerCameraPointer() ); }
    bool isGameLoaded() { return GetModuleHandleA( "halo1.dll" ) && isMapLoaded() && Halo1::isCameraLoaded() && areEntitiesLoaded(); }

}
