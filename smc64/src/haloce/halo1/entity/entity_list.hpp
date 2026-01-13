#pragma once

#include "../common.hpp"
#include "../tag.hpp"
#include <vector>
#include <functional>
#include "math/Vectors.hpp"

#include "../math.hpp"

#include "types.hpp"
#include "entity_functions.hpp"

namespace Halo1 {

    bool areEntitiesLoaded();
    
    Entity* getEntityPointer( EntityRecord* pRecord );
    Entity* getEntityPointer( uint32_t entityHandle );
    EntityRecord* getEntityRecord( uint32_t entityHandle );
    EntityRecord* getEntityRecord( EntityList* pEntityList, uint32_t entityHandle );
    void foreachEntityRecord( std::function<void( EntityRecord* )> cb );
}