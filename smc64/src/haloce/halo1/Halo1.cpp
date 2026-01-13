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

    // = BSP =======================

    uintptr_t getBSPPointer() {
        uintptr_t bspPointerPointer = (uintptr_t) ( dllBase() + 0x1C55C68U );
        uintptr_t bspPointer = Memory::safeRead<uintptr_t>(bspPointerPointer).value_or(0);
        return bspPointer;
    }

    uint32_t getBSPVertexCount() {
        uintptr_t bspPointer = getBSPPointer();
        if ( !bspPointer ) return 0;
        return Memory::safeRead<uint32_t>(bspPointer + 0x54 ).value_or( 0 );
    }

    BSPVertex* getBSPVertexArray() {
        uintptr_t bspPointer = getBSPPointer();
        if ( !bspPointer ) return nullptr;
        uint32_t vertexArrayAddress = Memory::safeRead<uint32_t>( bspPointer + 0x58 ).value_or( 0 );
        uintptr_t vertexArray = Halo1::translateMapAddress( vertexArrayAddress );
        if ( !vertexArray ) return nullptr;
        return (BSPVertex*) vertexArray;
    }

    uint32_t getBSPEdgeCount() {
        CollisionBSP* collisionBSP = (CollisionBSP*) getBSPPointer();
        if ( !collisionBSP || !Memory::isAllocated( (uintptr_t) collisionBSP ) )
            return 0;
        return collisionBSP->edges.count;
    }
    BSPEdge* getBSPEdgeArray() {
        CollisionBSP* collisionBSP = (CollisionBSP*) getBSPPointer();
        if ( !collisionBSP || !Memory::isAllocated( (uintptr_t) collisionBSP ) )
            return nullptr;
        uint64_t edgeArrayAddress = Halo1::translateMapAddress( collisionBSP->edges.offset );
        if ( !edgeArrayAddress ) return nullptr;
        return (BSPEdge*) edgeArrayAddress;
    }

    uint32_t getBSPPlaneCount() {
        CollisionBSP* collisionBSP = (CollisionBSP*) getBSPPointer();
        if ( !collisionBSP || !Memory::isAllocated( (uintptr_t) collisionBSP ) )
            return 0;
        return collisionBSP->planes.count;
    }
    BSPPlane* getBSPPlaneArray() {
        CollisionBSP* collisionBSP = (CollisionBSP*) getBSPPointer();
        if ( !collisionBSP || !Memory::isAllocated( (uintptr_t) collisionBSP ) )
            return nullptr;
        uint64_t planeArrayAddress = Halo1::translateMapAddress( collisionBSP->planes.offset );
        if ( !planeArrayAddress ) return nullptr;
        return (BSPPlane*) planeArrayAddress;
    }

    BSPSurface* getBSPSurfaceArray() {
        CollisionBSP* collisionBSP = (CollisionBSP*) getBSPPointer();
        if ( !collisionBSP || !Memory::isAllocated( (uintptr_t) collisionBSP ) )
            return nullptr;
        uint64_t surfaceArrayAddress = Halo1::translateMapAddress( collisionBSP->surfaces.offset );
        if ( !surfaceArrayAddress ) return nullptr;
        return (BSPSurface*) surfaceArrayAddress;
    }
    uint32_t getBSPSurfaceCount() {
        CollisionBSP* collisionBSP = (CollisionBSP*) getBSPPointer();
        if ( !collisionBSP || !Memory::isAllocated( (uintptr_t) collisionBSP ) )
            return 0;
        return collisionBSP->surfaces.count;
    }

    // = Collision Geometry ==========

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

        ObjectCollisionTagData* collisionData = (ObjectCollisionTagData*) tagData;
        if ( nodeIndex >= collisionData->collisionNodes.count ) return nullptr;

        return collisionData->collisionNodes.get<CollisionNode>( nodeIndex );
    }

    CollisionBSP* getObjectCollisionBSP( Tag* objTag ) {
        Tag* collisionTag = getCollisionGeometryTag( objTag );
        if ( !collisionTag ) return nullptr;

        ObjectCollisionTagData* tagData = (ObjectCollisionTagData*) collisionTag->getData();
        if ( !tagData ) return nullptr;

        CollisionNode* node0 = tagData->collisionNodes.get<CollisionNode>( 0 );
        if ( !node0 ) return nullptr;
        
        // We cannot provide an index here yet because we do not know the true size of this structure.
        return node0->collisionBsps.get<CollisionBSP>( 0 );
    }
}
