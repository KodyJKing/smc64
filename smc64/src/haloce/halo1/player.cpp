#include "player.hpp"

namespace Halo1 {

    Camera* getPlayerCameraPointer() { return (Camera*) ( dllBase() + 0x2D9B9C0 ); }

    bool isCameraLoaded() { return Memory::isAllocated( (uintptr_t) getPlayerCameraPointer() ); }

    uint32_t getPlayerHandle() { return *(uint32_t*) ( dllBase() + 0x1C563F0U ); }
    PlayerController* getPlayerControllerPointer() { return * (PlayerController**) ( dllBase() + 0x2D8FE70U ); }

    EntityRecord* getPlayerRecord() {
        auto rec = getEntityRecord( getPlayerHandle() );
        if ( !rec || !rec->entity() )
            return nullptr;
        return rec;
    }

    std::optional<Vec3> getPlayerPosition() {
        auto rec = getPlayerRecord();
        if ( !rec )
            return std::nullopt;
        auto entity = rec->entity();
        if ( !entity )
            return std::nullopt;
        return entity->pos;
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

}
