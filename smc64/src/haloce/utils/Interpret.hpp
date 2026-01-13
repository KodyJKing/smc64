#pragma once
#include "haloce/halo1/Halo1.hpp"

namespace Halo1 {
    struct Interpretations {
        Tag* tag = nullptr;
        Entity* entity = nullptr;
        uintptr_t mapPointer = 0;
    };

    Interpretations interpretU32( uint32_t value );

    bool hasInterpretation(uint32_t value);
}
