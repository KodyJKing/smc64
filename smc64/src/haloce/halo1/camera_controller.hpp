#pragma once

#include "stdint.h"

namespace Halo1 {
    #pragma pack(push, 1)
    struct CameraController {
        uint32_t flags; // D328263C for 1rst person, D3282CA4 for 3rd person
        char pad1[16];
    };
    #pragma pack(pop)    
}
