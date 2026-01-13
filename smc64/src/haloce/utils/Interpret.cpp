#include "haloce/utils/Interpret.hpp"
#include "haloce/halo1/halo1.hpp"
#include "memory/Memory.hpp"

namespace Halo1 {
    Interpretations interpretU32( uint32_t value ) {
        Interpretations result;

        if (!(value & 0xFF000000))
            return result; // Probably not a pointerlike value.
        
        // Maybe it is an entity handle?
        auto rec = getEntityRecord( value );
        if ( rec ) {
            auto entity = rec->entity();
            // Is this address allocated?
            if ( entity && Memory::isAllocated( (uintptr_t) entity ) ) {

                // Is the entity's tag valid?
                auto tag = entity->tag();
                if ( tag && tagExists( tag ) && validTagPath( tag->getResourcePath() ) ) {
                    result.tag = tag;
                }

                result.entity = entity;
            }
        }

        // Maybe it is a tag ID?
        auto tag = getTag( value );
        if ( tag && tagExists( tag ) && validTagPath( tag->getResourcePath() ) ) {
            result.tag = tag;
        }

        // Maybe it is a map address?
        uintptr_t mapPointer = translateMapAddress( value );
        if ( mapPointer && Memory::isAllocated( mapPointer ) ) {
            result.mapPointer = mapPointer;
        }

        return result;
    }

    bool hasInterpretation( uint32_t value ) {
        auto result = interpretU32( value );
        return result.tag || result.entity || result.mapPointer != 0;
    }
}
