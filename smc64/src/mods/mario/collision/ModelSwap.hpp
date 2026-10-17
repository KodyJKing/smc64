#pragma once

#include <vector>
#include <cstddef>

#include "libsm64.h"
#include "engine/halo1.hpp"

namespace Mod::Mario::DynamicGeometry::ModelSwap {

    // Returns true if a model swap is registered for the given entity/bone.
    bool hasModelSwapFor(Engine::Entity* entity, size_t boneIndex);

    // Appends the SM64 surfaces for the registered model swap (if any) to outSurfaces.
    // Surfaces are expressed in bone-local mario space.
    void getModelSwapSurfacesFor(Engine::Entity* entity, size_t boneIndex, std::vector<SM64Surface>& outSurfaces);

}
