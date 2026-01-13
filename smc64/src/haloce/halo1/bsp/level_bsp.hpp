#pragma once
#include <stdint.h>
#include "bsp.hpp"

namespace Halo1 {
    uintptr_t getBSPPointer();
    
    uint32_t getBSPVertexCount();
    BSPVertex* getBSPVertexArray();

    uint32_t getBSPEdgeCount();
    BSPEdge* getBSPEdgeArray();

    uint32_t getBSPPlaneCount();
    BSPPlane* getBSPPlaneArray();

    BSPSurface* getBSPSurfaceArray();
    uint32_t getBSPSurfaceCount();
}
