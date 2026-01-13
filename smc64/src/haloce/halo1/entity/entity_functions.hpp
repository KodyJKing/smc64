#pragma once

#include "types.hpp"

namespace Halo1 {
    bool isReloading( Entity* entity );
    bool isDoingMelee( Entity* entity );
    bool isTransport( Entity* entity );
    bool isRidingTransport( Entity* entity );
    uint16_t boneCount(void * anim);
}
