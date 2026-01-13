#include "entity.hpp"
#include "memory/Memory.hpp"

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

}
