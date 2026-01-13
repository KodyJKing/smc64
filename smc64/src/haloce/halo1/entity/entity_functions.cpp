#include "entity_functions.hpp"
#include "entity_list.hpp"

#include "memory/Memory.hpp"

namespace Halo1 {

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

        uint16_t boneCount(void* anim) {
        // Todo: Create a type for anim.
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
}
