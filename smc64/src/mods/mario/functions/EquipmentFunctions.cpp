#include "EquipmentFunctions.hpp"
#include "engine/halo1.hpp"

namespace Mod::Mario {

    void removeEquipmentEntity(Engine::Entity* entity) {
        // In liu of a propery way to do this, just teleport the entity way under the map.
        if (entity) {
            entity->pos.y = -1000.0f;
        }
    }

}
