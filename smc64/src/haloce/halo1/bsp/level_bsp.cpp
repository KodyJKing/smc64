#include "level_bsp.hpp"

namespace Halo1 {
 
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

}
