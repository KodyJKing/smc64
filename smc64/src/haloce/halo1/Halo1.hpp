// Reverse engineered Halo 1 structures and access functions go here.

#pragma once

#include <stdint.h>
#include <functional>
#include <string>
#include <vector>
#include "math/Vectors.hpp"

#include "memory/Memory.hpp"

#include "common.hpp"
#include "math.hpp"
#include "tag.hpp"
#include "entity/index.hpp"
#include "camera.hpp"
#include "player.hpp"
#include "map.hpp"

#define NULL_HANDLE 0xFFFFFFFF

namespace Halo1 {

    #pragma pack(push, 1)
        struct ArrayPointer {
            uint32_t count;
            uint32_t offset; // Use translateMapAddress to get the actual pointer
        };
        
        struct WeaponTagData {
            char pad_0000[0x4FC];
            ArrayPointer projectileData;
        };

        struct WeaponProjectileData {
            char pad_0000[0x4];
            float minRateOfFire;
            float maxRateOfFire;
            char pad_000C[0x108];
        }; // Size = 0x114
    #pragma pack(pop)

    // Created with ReClass.NET 1.2 by KN4CK3R
    class ProjectileTagData
    {
        public:
        char pad_0000[444]; //0x0000
        float minExplodeTime; //0x01BC
        float maxExplodeTime; //0x01C0
        char pad_01C4[4]; //0x01C4
        float lifeSpan; //0x01C8
        float arc; //0x01CC
        char pad_01D0[20]; //0x01D0
        float initialSpeed; //0x01E4
        float finalSpeed; //0x01E8
        float homing; //0x01EC
    }; //Size: 0x01F0

    WeaponProjectileData * getProjectileData(Tag * tag, uint32_t projectileIndex);

    extern float fovScale;
    extern float clippingNear;
    extern float clippingFar;
    // HRESULT getCameraMatrix( float w, float h, XMMATRIX& result );
    Vec3 projectPoint( float w, float h, const Vec3 point );

    bool isCameraLoaded();
    bool isGameLoaded();

    // = BSP =======================

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

    struct BlockPointer {
        uint32_t count;
        uint32_t offset;   // Use translateMapAddress to get the actual pointer
        uint32_t bullshit; // Not sure what this does.

        template <typename T>
        T* get(size_t index, bool safe = true) {
            if (index >= count) return nullptr;
            uint64_t baseAddress = Halo1::translateMapAddress(offset);
            if (!baseAddress) return nullptr;
            if (safe && !Memory::isAllocated(baseAddress + index * sizeof(T))) return nullptr;
            return (T*)(baseAddress + index * sizeof(T));
        }
    };

    struct CollisionBSP {
        BlockPointer bsp3DNodes, planes, leaves, bsp2DRefs, bsp2DNodes, surfaces, edges, vertices;
    };
    #pragma pack(pop)

    uintptr_t getBSPPointer();
    uint32_t getBSPVertexCount();
    BSPVertex* getBSPVertexArray();

    uint32_t getBSPEdgeCount();
    BSPEdge* getBSPEdgeArray();

    uint32_t getBSPPlaneCount();
    BSPPlane* getBSPPlaneArray();

    BSPSurface* getBSPSurfaceArray();
    uint32_t getBSPSurfaceCount();

    // = Collision Geometry ==========

    struct ObjectCollisionTagData {
        char pad[0x28C];
        BlockPointer collisionNodes;
    };
    static_assert(offsetof(ObjectCollisionTagData, collisionNodes.offset) == 0x290, "ObjectCollisionTagData::collisionNodes.count offset is not 0x290 bytes");

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
