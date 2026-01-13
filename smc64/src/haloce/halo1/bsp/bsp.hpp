#pragma once

#include <stdint.h>
#include "math/Vectors.hpp"
#include "memory/Memory.hpp"
#include "../common.hpp"
#include "../map.hpp"
#include "../tag.hpp"

namespace Halo1 {
    #pragma pack(push, 1)
    struct BSPVertex {
        Vec3 pos;
        uint32_t firstEdgeIndex;
    };

    struct BSPEdge {
        uint32_t startVertex, endVertex;    // Indices into the BSPVertex array
        uint32_t forwardEdge, backwardEdge; // Indices into the BSPEdge array
        uint32_t leftSurface, rightSurface; // Indices into the BSPSurface array
    };

    struct BSPPlane {
        Vec3 normal; // Normal vector of the plane
        float distance; // Distance from the origin to the plane
    };

    struct BSPSurface {
        uint32_t planeIndex; // Index into the BSPPlane array
        uint32_t firstEdgeIndex; // Index into the BSPEdge array
        uint8_t flags;
        uint8_t breakableSurface;
        uint16_t material;
    };
    static_assert(sizeof(BSPSurface) == 12, "BSPSurface must be 12 bytes long");

    struct CollisionBSP {
        BlockPointer bsp3DNodes, planes, leaves, bsp2DRefs, bsp2DNodes, surfaces, edges, vertices;
    };
    #pragma pack(pop)
}