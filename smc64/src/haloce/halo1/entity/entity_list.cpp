#include "entity_list.hpp"
#include "memory/Memory.hpp"
#include <iostream>

namespace Halo1 {

    Tag* Entity::tag() { return Halo1::getTag( tagID ); }
    char* Entity::getTagResourcePath() {
        auto pTag = tag();
        if ( !pTag ) return nullptr;
        return pTag->getResourcePath();
    };
    bool Entity::fromResourcePath( const char* str ) {
        auto resourcePath = getTagResourcePath();
        return resourcePath && strncmp( resourcePath, str, 1024 ) == 0;
    }

    Transform* Entity::getBoneTransforms() {
        if ( !bonesOffset ) return nullptr;
        return (Transform*) ( (uintptr_t) this + bonesOffset );
    }

    std::vector<Transform> Entity::copyBoneTransforms() {
        std::vector<Transform> result;
        auto boneCount = this->boneCount();
        if ( !boneCount ) return result;
        auto bones = this->getBoneTransforms();
        for ( uint16_t i = 0; i < boneCount; i++ )
            result.push_back( bones[i] );
        return result;
    }

    Entity* EntityRecord::entity() { return getEntityPointer( this ); }

    EntityList* getEntityListPointer() { return *(EntityList**) ( dllBase() + 0x1C42248 ); }
    uintptr_t getEntityArrayBase() { return *(uintptr_t*) ( dllBase() + 0x2D9CDF8 ); }

    bool isEntityListLoaded() { return Memory::isAllocated( (uintptr_t) getEntityListPointer() ); }
    bool isEntityArrayLoaded() { return Memory::isAllocated( (uintptr_t) getEntityArrayBase() ); }
    bool areEntitiesLoaded() { return isEntityListLoaded() && isEntityArrayLoaded(); }

    EntityRecord* getEntityRecord( EntityList* pEntityList, uint32_t entityHandle ) {
        if ( entityHandle == 0xFFFFFFFF ) return nullptr;
        uint32_t i = entityHandle & 0xFFFF;
        auto recordAddress = (uintptr_t) pEntityList + pEntityList->entityListOffset + i * sizeof( EntityRecord );
        return (EntityRecord*) recordAddress;
    }
    EntityRecord* getEntityRecord( uint32_t entityHandle ) { return getEntityRecord( getEntityListPointer(), entityHandle ); }

    Entity* getEntityPointer( EntityRecord* pRecord ) {
        if ( !pRecord || pRecord->entityArrayOffset == -1 ) return nullptr;
        uintptr_t entityAddress = getEntityArrayBase() + 0x34 + (INT_PTR) pRecord->entityArrayOffset;
        return (Entity*) entityAddress;
    }

    Entity* getEntityPointer( uint32_t entityHandle ) {
        return getEntityPointer( getEntityRecord( entityHandle ) );
    }

    uint32_t indexToEntityHandle( uint16_t index ) {
        auto rec = getEntityRecord( index );
        if ( !rec ) return 0xFFFFFFFF;
        return rec->id << 16 | index;
    }

    void foreachEntityRecordIndexed( std::function<void( EntityRecord*, uint16_t i )> cb ) {
        auto pEntityList = getEntityListPointer();
        if ( !pEntityList ) return;
        for ( uint16_t i = 0; i < pEntityList->capacity; i++ ) {
            auto pRecord = getEntityRecord( pEntityList, i );
            if ( pRecord->entityArrayOffset == -1 ) continue;
            cb( pRecord, i );
        }
    }

    void foreachEntityRecord( std::function<void( EntityRecord* )> cb ) {
        foreachEntityRecordIndexed( [&cb]( EntityRecord* rec, uint16_t i ) { cb( rec ); } );
    }

}
