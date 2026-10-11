#include "KillPlayer.hpp"

#include "engine/halo1.hpp"
#include "spark/hook/Hooks.hpp"

namespace Mod::Mario {

    void damagePlayer(float amount, float multiplier) {
        if (!Spark::DamageEntity::original) return;

        auto* tag = Engine::findTag("weapons\\plasma rifle\\bolt", "jpt!");
        if (!tag) return;

        uint32_t playerHandle = Engine::getPlayerHandle();
        auto* player = Engine::getEntityPointer(playerHandle);
        if (!player) return;

        Engine::DamageEvent ev{};
        ev.damageTypeTagHandle = tag->tagID;
        ev.flags               = 0x1; // single-entity target
        ev.interactorHandle    = NULL_HANDLE;
        ev.attackerHandle      = NULL_HANDLE;
        ev.sourceTypeIndex     = (uint16_t)-1;
        ev.hitPosition         = player->pos;
        ev.hitDirection        = Vec3{ 0.0f, 0.0f, -1.0f };
        ev.baseDamage          = amount;
        ev.damageMultiplier    = multiplier;
        Spark::DamageEntity::dispatch(&ev, playerHandle, 0, 0, -1, 0);
    }

    void killPlayer() {
        damagePlayer(100.0f, 100.0f);
    }

}
