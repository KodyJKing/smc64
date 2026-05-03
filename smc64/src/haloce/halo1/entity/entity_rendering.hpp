#pragma once

#include "stdint.h"

namespace Halo1 {
    struct RenderEntityRequest {
        uint32_t entityHandle;
        uint32_t padding[31];
    };
    
    #define RENDER_ENTITY_FUNC_OFFSET 0xB48CD0U
    typedef void (*renderEntity_t)(RenderEntityRequest* request);
}
