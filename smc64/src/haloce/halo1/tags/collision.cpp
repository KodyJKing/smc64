#include "collision.hpp"

namespace Halo1 {

    uint32_t collisionGeometryTagId( Tag* objectTag ) {
        if ( !objectTag || !Memory::isAllocated( (uintptr_t) objectTag ) ) return NULL_HANDLE;
        // The collision geometry tag ID is stored at offset 0x7C in the object tag data.
        // Todo: Create a type for object tag data.
        return Memory::safeRead<uint32_t>( (uintptr_t) objectTag->getData() + 0x7C ).value_or( NULL_HANDLE );
    }

    Tag* getCollisionGeometryTag( Tag* objectTag ) {
        uint32_t collisionTagId = collisionGeometryTagId( objectTag );
        if ( collisionTagId == NULL_HANDLE ) return nullptr;
        Tag* collisionTag = getTag( collisionTagId );
        if ( !collisionTag || !Memory::isAllocated( (uintptr_t) collisionTag ) ) return nullptr;
        return collisionTag;
    }

    CollisionNode* getObjectCollisionNode( Tag* objTag, size_t nodeIndex ) {
        Tag* collisionTag = getCollisionGeometryTag( objTag );
        if ( !collisionTag ) return nullptr;

        void* tagData = collisionTag->getData();
        if ( !tagData ) return nullptr;

        CollisionTagData* collisionData = (CollisionTagData*) tagData;
        if ( nodeIndex >= collisionData->collisionNodes.count ) return nullptr;

        return collisionData->collisionNodes.get<CollisionNode>( nodeIndex );
    }

    CollisionBSP* getObjectCollisionBSP( Tag* objTag ) {
        Tag* collisionTag = getCollisionGeometryTag( objTag );
        if ( !collisionTag ) return nullptr;

        CollisionTagData* tagData = (CollisionTagData*) collisionTag->getData();
        if ( !tagData ) return nullptr;

        CollisionNode* node0 = tagData->collisionNodes.get<CollisionNode>( 0 );
        if ( !node0 ) return nullptr;
        
        // We cannot provide an index here yet because we do not know the true size of this structure.
        return node0->collisionBsps.get<CollisionBSP>( 0 );
    }

}
