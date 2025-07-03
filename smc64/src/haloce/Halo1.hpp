// Reverse engineered Halo 1 structures and access functions go here.

#pragma once

#include <stdint.h>
#include <functional>
#include <string>
#include <vector>
#include "math/Vectors.hpp"

#define NULL_HANDLE 0xFFFFFFFF

namespace Halo1 {

    // Thanks to Kavawuvi for documentation on the map format and tag structure.
    class Tag {
        public:
        uint32_t groupID; // Group ID's are fourcc's
        uint32_t parentGroupID; 
        uint32_t grandparentGroupID;
        uint32_t tagID; 
        uint32_t resourcePathAddress; 
        uint32_t dataAddress; 
        char pad_0018[8]; 

        char* getResourcePath();
        void* getData();
        std::string groupIDStr();
    };

    #pragma pack(push, 1)
        struct MapHeader {
            uint32_t magicHeader; // Should be 1751474532 ('head' in ascii fourcc)
            uint32_t cacheVersion;
            uint32_t fileSize;
            uint32_t paddingLength; // Only used on Xbox
            uint32_t tagDataOffset;
            uint32_t tagDataSize;
            char pad0[8];
            char mapName[32];
            char buildVersion[32];
            uint32_t scenarioType;
            uint32_t checksum;
            char pad1[0x794];
            uint32_t magicFooter; // Should be 1718579060 ('foot' in ascii fourcc)
        };

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

        struct Camera {
            char pad0[4];
            Vec3 pos;
            char pad1[16];
            float fov;
            Vec3 fwd;
            Vec3 up;
        };

        struct Transform {
            Quaternion rotation;
            Vec3 translation;
            float scale;
        };

        namespace PlayerActionFlags {
            const uint32_t crouch     = 1 << 0;
            const uint32_t jump       = 1 << 1;
            const uint32_t flash      = 1 << 4;
            const uint32_t melee      = 1 << 7;
            const uint32_t reload     = 1 << 10;
            const uint32_t shoot      = 1 << 11;
            const uint32_t grenade1   = 1 << 12;
            const uint32_t grenade2   = 1 << 13;
        }
        struct PlayerController {
            uint8_t pad_0000[16];  // 0000
            uint32_t playerHandle; // 0010
            uint32_t actions;      // 0014
            uint8_t pad_0018[4];   // 0018
            float yaw;             // 001C
            float pitch;           // 0020
            float walkY;           // 0024
            float walkX;           // 0028
            float gunTrigger;      // 002C
            uint8_t pad_0030[8];   // 0030
            float targetIndicator; // 0038
            uint8_t pad_003C[360]; // 003C
            uint32_t targetHandle; // 01A4
        };
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

    class EntityList {
        public:
        char pad_0000[32]; 
        uint16_t capacity; 
        char pad_0022[14]; 
        uint16_t count; 
        int32_t entityListOffset; 
    };

    class Entity {
        public:
        uint32_t tagID; 
        char pad_0004[16]; 
        uint32_t ageMilis; 
        Vec3 pos; 
        Vec3 vel; 
        Vec3 fwd; 
        Vec3 up; 
        Vec3 angularVelocity; 
        char pad_0054[8]; 
        Vec3 rootBonePos; 
        char pad_0068[8]; 
        uint16_t entityCategory; 
        char pad_0072[14]; 
        uint32_t controllerHandle; 
        char pad_0084[4];
        uint32_t animSetTagID;
        uint16_t animId; 
        uint16_t animFrame; 
        char pad_0090[12]; 
        float health; 
        float shield; 
        char pad_00A4[44]; 
        uint32_t vehicleHandle; 
        uint32_t childHandle; 
        uint32_t parentHandle;
        char pad_00DC[204];
        uint16_t bonesByteCount;
        uint16_t bonesOffset;
        char pad_01AC[84];
        uint32_t projectileParentHandle; 
        float heat; 
        float plasmaUsed; 
        float fuse; 
        char pad_0210[12]; 
        float projectileAge; 
        char pad_0220[8]; 
        uint8_t ticksSinceLastFired; 
        char pad_0229[23]; 
        float plasmaCharge; 
        char pad_0244[61]; 
        uint8_t weaponIndex; 
        char pad_0282[1]; 
        uint8_t grenadeAnim; 
        uint8_t weaponAnim; 
        char pad_0285[1]; 
        uint16_t ammo; 
        char pad_0288[2]; 
        uint16_t clipAmmo; 
        char pad_028C[112]; 
        uint8_t frags; 
        uint8_t plasmas; 
        char pad_02FE[6]; 
        uint32_t vehicleRiderHandle;

        Tag* tag();
        char* getTagResourcePath();
        bool fromResourcePath( const char* str );

        uint16_t boneCount();
        Transform* getBoneTransforms();
        std::vector<Transform> copyBoneTransforms();
    };

    class EntityRecord {
        public:
        uint16_t id;
        uint16_t unknown_1;
        uint16_t unknown_2;
        uint16_t typeId;
        int32_t entityArrayOffset;

        Entity* entity();
    };

    // =======================================================

    enum EntityCategory {
        EntityCategory_Biped,
        EntityCategory_Vehicle,
        EntityCategory_Weapon,
        EntityCategory_Equipment,
        EntityCategory_Garbage,
        EntityCategory_Projectile,
        EntityCategory_Scenery,
        EntityCategory_Machine,
        EntityCategory_Control,
        EntityCategory_LightFixture,
        EntityCategory_Placeholder,
        EntityCategory_SoundScenery,
    };

    enum TypeID {
        TypeID_Player = 0x0DE4,
        TypeID_Marine = 0x0E58,
        TypeID_Jackal = 0x1184,
        TypeID_Grunt = 0x0CFC,
        TypeID_Elite = 0x1110,
        TypeID_VehicleA = 0x0AF4,
        TypeID_VehicleB = 0x06E0,
        TypeID_Projectile = 0x0290,
    };

    // =======================================================

    EntityList* getEntityListPointer();
    Camera* getPlayerCameraPointer();
    uint32_t getPlayerHandle();
    PlayerController * getPlayerControllerPointer();
    char* getMapName();
    MapHeader* getMapHeader();

    bool isOnMap( const char* mapName );

    Tag* getTag( uint32_t tagID );
    Tag * findTag(const char * path, const char * fourCC);
    Tag * findTag(const char * path, uint32_t fourCC);
    bool validTagPath(const char * path);
    bool tagExists(Tag * tag);

    uint64_t translateMapAddress( uint32_t address );

    uint32_t translateToMapAddress(uint64_t absoluteAddress);

    WeaponProjectileData * getProjectileData(Tag * tag, uint32_t projectileIndex);

    uint16_t boneCount(void * anim);

    void foreachEntityRecord( std::function<void( EntityRecord* )> cb );
    void foreachEntityRecordIndexed( std::function<void( EntityRecord*, uint16_t i )> cb );

    Entity* getEntityPointer( EntityRecord* pRecord );
    Entity* getEntityPointer( uint32_t entityHandle );
    EntityRecord* getEntityRecord( uint32_t entityHandle );
    EntityRecord* getEntityRecord( EntityList* pEntityList, uint32_t entityHandle );

    uint32_t indexToEntityHandle( uint16_t index );

    EntityRecord* getPlayerRecord();

    bool isPlayerHandle( uint32_t entityHandle );
    bool isPlayerControlled( EntityRecord* rec );

    bool isReloading( Entity* entity );
    bool isDoingMelee( Entity* entity );

    bool isTransport( Entity* entity );
    bool isRidingTransport( Entity* entity );

    bool printEntity( EntityRecord* pRecord );
    void printEntities();

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

    struct BlockPointer {
        uint32_t count;
        uint32_t offset;   // Use translateMapAddress to get the actual pointer
        uint32_t bullshit; // Not sure what this does.
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
}