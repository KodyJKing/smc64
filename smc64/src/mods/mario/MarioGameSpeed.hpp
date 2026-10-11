#pragma once

#include "engine/halo1.hpp"

namespace Mod::Mario {
    void updateGameSpeed(Engine::Entity& player);
    void grantFreeBulletTime(uint64_t durationMillis);
}
