#pragma once

#include "stdint.h"
#include "decomp/sm64.h"

namespace Mod::Mario {

    uint32_t sanitizePersistedAction(uint32_t action) {
        bool ridingShell = (action & ACT_FLAG_RIDING_SHELL) != 0;
        if (ridingShell) {
            return ACT_RIDING_SHELL_GROUND;
        }
        return ACT_IDLE;
    }
    
}
