#include "IsElevatorEntity.hpp"

#include <cstring>

// #include <iostream>

namespace Mod::Mario {

    bool isElevatorTag(Engine::Tag* tag) {
        if (!tag) return false;
        const char* path = tag->getResourcePath();
        if (!path) return false;

        return strstr(path, "elevator") != nullptr
            || strstr(path, "platform") != nullptr
            || strstr(path, "lift") != nullptr;
    }

    bool isElevatorEntity(Engine::Entity* entity) {
        auto tag = entity->tag();
        if (!tag) return false;
        return isElevatorTag(tag);
    }

}
