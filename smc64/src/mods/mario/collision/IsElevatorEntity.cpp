#include "IsElevatorEntity.hpp"
#include "memory/Memory.hpp"
#include <cstring>

namespace Mod::Mario {

    bool isElevatorTag(Engine::Tag* tag) {
        // Todo: Replace isAllocatedFast checks with bounds checks after merging tag-injection PR.
        if ( !tag || !Memory::isAllocatedFast((uintptr_t)tag) ) return false;
        const char* path = tag->getResourcePath();
        if ( !path || !Memory::isAllocatedFast((uintptr_t)path) ) return false;
        return strstr(path, "elevator") != nullptr
            || strstr(path, "platform") != nullptr
            || strstr(path, "lift") != nullptr;
    }

    bool isElevatorEntity(Engine::Entity* entity) {
        if (!entity) return false;
        if (!Engine::entityValid(entity)) return false;
        auto tag = entity->tag();
        if (!tag) return false;
        return isElevatorTag(tag);
    }

}
