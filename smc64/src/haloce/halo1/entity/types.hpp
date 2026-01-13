#pragma once

#include <stdint.h>
#include <vector>
#include "math/Vectors.hpp"
#include "../math.hpp"
#include "../tag.hpp"

namespace Halo1 {

    template<typename T>
    struct ChildArrayHeader {
        uint16_t byteCount;
        uint16_t offset;
        uint16_t count() { return byteCount / sizeof(T); }
        T* get( void* parent, uint16_t index ) {
            if ( offset == 0 ) return nullptr;
            return (T*) ( (uintptr_t) parent + offset + index * sizeof(T) );
        }
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
        ChildArrayHeader<Transform> bones;
        ChildArrayHeader<WorldTransform> worldBones;
        char pad_01B0[80];
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
        Transform* getBoneTransforms() { return bones.get( this, 0 ); }
        std::vector<Transform> copyBoneTransforms();
    };
    static_assert( offsetof(Entity, pad_01B0) == 0x1B0 );
    static_assert( offsetof(Entity, pad_02FE) == 0x2FE );

    class EntityRecord {
        public:
        uint16_t id;
        uint16_t unknown_1;
        uint16_t unknown_2;
        uint16_t typeId;
        int32_t entityArrayOffset;

        Entity* entity();
    };

    class EntityList {
        public:
        char pad_0000[32]; 
        uint16_t capacity; 
        char pad_0022[14]; 
        uint16_t count; 
        int32_t entityListOffset; 
    };

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
    
}