// Reverse engineered Halo 1 structures and access functions go here.

#include "Halo1.hpp"
#include <Windows.h>
#include <iostream>
#include "memory/Memory.hpp"
#include "utils/Strings.hpp"

namespace Halo1 {

    uintptr_t dllBase() {
        return (uintptr_t) GetModuleHandleA( "halo1.dll" );
    }

    // === Methods ===

    Entity* EntityRecord::entity() { return getEntityPointer( this ); }

    char* Tag::getResourcePath() { return (char*) translateMapAddress( resourcePathAddress ); }
    void* Tag::getData() { return (void*) translateMapAddress( dataAddress ); }
    std::string Tag::groupIDStr() {
        auto fourccA = Strings::fourccToString( groupID );
        auto fourccB = Strings::fourccToString( parentGroupID );
        auto fourccC = Strings::fourccToString( grandparentGroupID );
        return "[" + fourccC + " > " + fourccB + " > " + fourccA + "]";
    }

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

    uint16_t boneCount(void* anim) {
        return Memory::safeRead<uint16_t>( (uintptr_t) anim + 0x2c ).value_or( 0 );
    }

    uint16_t Entity::boneCount() {
        return bonesByteCount / sizeof( Transform );

        // This is a good example of how to traverse animation tag data, so I'm keeping it around.
        //
        // auto animSetTag = Halo1::getTag(animSetTagID);
        // if ( !animSetTag ) return 0;
        // void* animSetData = animSetTag->getData();
        // if ( !animSetData ) return 0;
        // uint32_t animArrayAddress = Memory::safeRead<uint32_t>( (uintptr_t) animSetData + 0x78 ).value_or( 0 );
        // uintptr_t animArray = Halo1::translateMapAddress( animArrayAddress );
        // if ( !animArray ) return 0;
        // int animIndex = 0;
        // size_t sizeOfAnimation = 0xb4;
        // uintptr_t anim = animArray + animIndex * sizeOfAnimation;
        // return Halo1::boneCount( (void*) anim );
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

    // === Pointers ===

    static const uintptr_t entityArrayOffset = 0x2D9CDF8;
    static const uintptr_t pEntityListOffset = 0x1C42248;
    static const uintptr_t playerCamOffset = 0x2D9B9C0;
    static const uintptr_t playerHandleOffset = 0x29AE480U;
    static const uintptr_t playerControllerOffset = 0x2D8FE70U;

    EntityList* getEntityListPointer() { return *(EntityList**) ( dllBase() + pEntityListOffset ); }
    uintptr_t getEntityArrayBase() { return *(uintptr_t*) ( dllBase() + entityArrayOffset ); }
    Camera* getPlayerCameraPointer() { return (Camera*) ( dllBase() + playerCamOffset ); }
    uint32_t getPlayerHandle() { return *(uint32_t*) ( dllBase() + playerHandleOffset ); }
    PlayerController* getPlayerControllerPointer() { return * (PlayerController**) ( dllBase() + playerControllerOffset ); }

    // No longer includes file path, only the map name.
    char* getMapName() {
        MapHeader* header = getMapHeader();
        if ( !header ) return nullptr;
        return header->mapName;
    }

    bool checkMapHeader(MapHeader* header) {
        if (!header) {
            // std::cout << "Error: header is null" << std::endl;
            return false;
        }
        if ( !Memory::isAllocated( (uintptr_t) header ) ) {
            // std::cout << "Error: header is not allocated" << std::endl;
            return false;
        }
        if (header->magicHeader != 1751474532) {
            // std::cout << "Error: magicHeader is not 1751474532" << std::endl;
            return false;
        }
        if (header->magicFooter != 1718579060) {
            // std::cout << "Error: magicFooter is not 1718579060" << std::endl;
            // std::cout << offsetof(MapHeader, magicFooter) << std::endl;
            return false;
        }
        return true;
    }

    MapHeader* getMapHeader() {
        MapHeader* result = (MapHeader*) ( dllBase() + 0x2B22744U );
        if ( !checkMapHeader( result ) )
            return nullptr;
        return result;
    }

    bool isOnMap( const char* mapName ) {
        auto actualMapName = getMapName();
        if ( !actualMapName )
            return false;
        return strncmp( mapName, actualMapName, strnlen( mapName, 32 ) ) == 0;
    }

    bool isMapLoaded() {
        auto header = getMapHeader();
        return header && Memory::isAllocated( (uintptr_t) header ) && checkMapHeader( header );
    }

    static const uintptr_t relocatedMapBaseOffset = 0x2D9CE10U;
    static const uintptr_t mapBaseOffset = 0x2EA3410U;

    uint64_t translateMapAddress( uint32_t address ) {
        uint64_t relocatedMapBase = *(uint64_t*) ( dllBase() + relocatedMapBaseOffset );
        uint64_t mapBase = *(uint64_t*) ( dllBase() + mapBaseOffset );
        return address + ( relocatedMapBase - mapBase );
    }

    uint32_t translateToMapAddress( uint64_t absoluteAddress ) {
        uint64_t relocatedMapBase = *(uint64_t*) ( dllBase() + relocatedMapBaseOffset );
        uint64_t mapBase = *(uint64_t*) ( dllBase() + mapBaseOffset );
        return (uint32_t) ( absoluteAddress - ( relocatedMapBase - mapBase ) );
    }

    Tag* getTag( uint32_t tagID ) {
        if (tagID == NULL_HANDLE)
            return nullptr;
        Tag* tagArray = *(Tag**) ( dllBase() + 0x1C34FB0U );
        return &tagArray[tagID & 0xFFFF];
    }

    Tag* findTag( const char* path, uint32_t fourCC) {
        if ( !validTagPath( path ) ) return nullptr;
        Tag* tagArray = *(Tag**) ( dllBase() + 0x1C34FB0U );
        for ( uint32_t i = 0; i < 0x10000; i++ ) {
            auto tag = &tagArray[i];
            if ( !tagExists( tag ) ) break;
            if ( strcmp( tag->getResourcePath(), path ) == 0 && tag->groupID == fourCC )
                return tag;
        }
        return nullptr;
    }

    Tag* findTag( const char* path, const char* fourCC ) {
        uint32_t fourCCValue = Strings::stringToFourcc( fourCC );
        return findTag( path, fourCCValue );
    }

    WeaponProjectileData* getProjectileData( Tag* tag, uint32_t projectileIndex) {
        if ( !tag ) return nullptr;
        auto data = (WeaponTagData*) tag->getData();
        if ( !data ) return nullptr;
        if (data->projectileData.count <= projectileIndex) return nullptr;
        auto projectileData = (WeaponProjectileData*) translateMapAddress( data->projectileData.offset );
        return &projectileData[projectileIndex];
    }

    bool validTagPath( const char* path ) {
        // Must match [a-zA-Z0-9_ \.\\-]+
        // Must be atleast 3 characters long.
        // Also must contain atleast one backslash.
        int backslashCount = 0;
        for ( size_t i = 0; i < 512; i++ ) {
            char c = path[i];
            if ( c == 0 ) 
                return backslashCount > 0 && i > 2;
            if ( 
                !(c >= 'a' && c <= 'z') &&
                !(c >= 'A' && c <= 'Z') &&
                !(c >= '0' && c <= '9') && 
                c != '_'   && c != ' '  &&
                c != '\\'  && c != '.'  && c != '-'
            )
                return false;
            if ( c == '\\' ) 
                backslashCount++;
        }
        return true;
    }
    
    bool tagExists( Tag* tag ) {
        return tag && Memory::isAllocated( (uintptr_t) tag->getData() ) && Memory::isAllocated( (uintptr_t) tag->getResourcePath() );
    }

    EntityRecord* getEntityRecord( EntityList* pEntityList, uint32_t entityHandle ) {
        if ( entityHandle == 0xFFFFFFFF )
            return nullptr;
        uint32_t i = entityHandle & 0xFFFF;
        auto recordAddress = (uintptr_t) pEntityList + pEntityList->entityListOffset + i * sizeof( EntityRecord );
        return (EntityRecord*) recordAddress;
    }
    EntityRecord* getEntityRecord( uint32_t entityHandle ) { return getEntityRecord( getEntityListPointer(), entityHandle ); }

    void foreachEntityRecordIndexed( std::function<void( EntityRecord*, uint16_t i )> cb ) {
        auto pEntityList = getEntityListPointer();
        if ( !pEntityList )
            return;
        for ( uint16_t i = 0; i < pEntityList->capacity; i++ ) {
            auto pRecord = getEntityRecord( pEntityList, i );
            if ( pRecord->entityArrayOffset == -1 )
                continue;
            cb( pRecord, i );
        }
    }

    void foreachEntityRecord( std::function<void( EntityRecord* )> cb ) {
        foreachEntityRecordIndexed( [&cb]( EntityRecord* rec, uint16_t i ) { cb( rec ); } );
    }

    Entity* getEntityPointer( EntityRecord* pRecord ) {
        if ( !pRecord || pRecord->entityArrayOffset == -1 )
            return nullptr;
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

    // ================

    EntityRecord* getPlayerRecord() {
        auto rec = getEntityRecord( getPlayerHandle() );
        if ( !rec || !rec->entity() )
            return nullptr;
        return rec;
    }

    bool isPlayerHandle( uint32_t entityHandle ) {
        auto rec = getEntityRecord( entityHandle );
        return rec && rec->typeId == TypeID_Player;
    }

    bool isPlayerControlled( EntityRecord* rec ) {
        auto entity = getEntityPointer( rec );
        if ( !entity )
            return false;

        return rec->typeId == TypeID_Player
            || isPlayerHandle( entity->parentHandle )
            // || isPlayerHandle( entity->vehicleRiderHandle )
            // || isPlayerHandle( entity->controllerHandle )
            // || isPlayerHandle( entity->projectileParentHandle )
            // || rec->typeId == 0x0454
            ;
    }

    bool isReloading( Entity* entity ) { return entity->weaponAnim == 0x05; }
    bool isDoingMelee( Entity* entity ) { return entity->weaponAnim == 0x07; }

    bool isTransport( Entity* entity ) {
        return
            entity->fromResourcePath( "vehicles\\pelican\\pelican" ) ||
            entity->fromResourcePath( "vehicles\\c_dropship\\c_dropship" );
    }

    bool isRidingTransport( Entity* entity ) {
        if ( !entity )
            return false;
        auto vehicleRec = getEntityRecord( entity->parentHandle );
        if ( !vehicleRec )
            return false;
        auto vehicle = getEntityPointer( vehicleRec );
        return isTransport( vehicle );
    }

    // ======================

    bool printEntity( EntityRecord* pRecord ) {
        auto pEntity = getEntityPointer( pRecord );
        if ( !pEntity )
            return true;
        std::cout << "Position: ";
        std::cout << pEntity->pos.toString() << std::endl;
        std::cout << "Type ID: " << pRecord->typeId;
        std::cout << ", Health: " << pEntity->health;
        std::cout << ", Shield: " << pEntity->shield << std::endl << std::endl;
        return true;
    }

    void printEntities() {
        auto pEntityList = getEntityListPointer();
        if ( pEntityList ) {
            std::cout << "Entity list at: " << pEntityList << "\n";
            std::cout << "Entities: \n\n";
            foreachEntityRecord( []( EntityRecord* rec ) { printEntity( rec ); } );
        }
    }

    // =======================

    bool isEntityListLoaded() { return Memory::isAllocated( (uintptr_t) getEntityListPointer() ); }
    bool isEntityArrayLoaded() { return Memory::isAllocated( (uintptr_t) getEntityArrayBase() ); }
    bool isCameraLoaded() { return Memory::isAllocated( (uintptr_t) getPlayerCameraPointer() ); }
    bool isGameLoaded() { return GetModuleHandleA( "halo1.dll" ) && isMapLoaded() && Halo1::isCameraLoaded() && isEntityListLoaded() && isEntityArrayLoaded(); }

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

}