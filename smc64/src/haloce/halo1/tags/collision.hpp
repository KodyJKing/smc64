#pragma once

#include <stdint.h>
#include <string>

#include "memory/Memory.hpp"

#include "../tag.hpp"
#include "../bsp/index.hpp"

namespace Halo1 {

    struct CollisionTagData {
        char pad[0x28C];
        BlockPointer collisionNodes;
    };
    static_assert(offsetof(CollisionTagData, collisionNodes.offset) == 0x290, "CollisionTagData::collisionNodes.count offset is not 0x290 bytes");

    struct CollisionNode {
        char name[0x2C];
        uint16_t region;
        uint16_t parentNode;
        uint16_t nextSiblingNode;
        uint16_t firstChildNode;
        BlockPointer collisionBsps;
    };
    static_assert(sizeof(CollisionNode) == 0x40, "CollisionNode size is not 0x40 bytes");
    static_assert(offsetof(CollisionNode, collisionBsps.offset) == 0x38, "CollisionNode::collisionBsps offset is not 0x38 bytes");

    uint32_t collisionGeometryTagId(Tag* objectTag);

    Tag* getCollisionGeometryTag( Tag* objectTag );

    CollisionBSP* getObjectCollisionBSP( Tag* objectCollTag );
}
