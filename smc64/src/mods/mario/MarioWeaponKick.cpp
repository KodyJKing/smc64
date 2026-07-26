#include "MarioWeaponKick.hpp"

#include "MarioState.hpp"
#include "engine/halo1.hpp"
#include "engine/entity/entity_list.hpp"
#include "spark/hook/Hooks.hpp"
#include "decomp/sm64.h"

#include <string>
#include <unordered_map>

namespace Mod::Mario::MarioWeaponKick {

    // ── Config ─────────────────────────────────────────────────────────────────

    struct KickConfig {
        float speed; // launch speed in mario units/tick
        std::string projectileType;
    };

    // Add entries here to enable kick for a weapon. Tag paths use double backslashes.
    static const std::unordered_map<std::string, KickConfig> kickMap = {
        { "weapons\\shotgun\\shotgun", { 60.0f, "weapons\\shotgun\\pellet" } },
    };

    // ── Debounce state ─────────────────────────────────────────────────────────
    // Many weapons spawn multiple projectiles per trigger pull (e.g. shotgun pellets).
    // We track the last game tick on which a kick was applied and suppress duplicates.

    static uint32_t sLastKickTick = UINT32_MAX;

    // ── Helpers ────────────────────────────────────────────────────────────────

    static const KickConfig* kickConfigForWeapon(uint32_t weaponHandle) {
        if (weaponHandle == 0xFFFFFFFF) return nullptr;
        auto* rec = Engine::getEntityRecord(weaponHandle);
        if (!rec) return nullptr;
        auto* ent = rec->entity();
        if (!ent) return nullptr;
        const char* path = ent->getTagResourcePath();
        if (!path) return nullptr;
        auto it = kickMap.find(path);
        return (it != kickMap.end()) ? &it->second : nullptr;
    }

    // Applies a backwards horizontal + upward velocity impulse to Mario.
    // Direction is derived from the camera's horizontal facing so it's consistent
    // regardless of how much the player is aiming up or down.
    static void applyKick(const KickConfig& cfg) {
        auto* camera = Engine::getPlayerCameraPointer();
        if (!camera) return;

        // Halo → mario axis mapping: mario.x = halo.x, mario.y = halo.z (up), mario.z = halo.y.
        // Kick is the exact opposite of the camera look direction.
        Vec3 f = camera->fwd.normalize();
        sm64_set_mario_velocity(Mario::marioId,
            -f.x * cfg.speed + Mario::marioState.velocity[0],
            -f.z * cfg.speed + Mario::marioState.velocity[1],
            -f.y * cfg.speed + Mario::marioState.velocity[2]);
    }

    // ── Public ─────────────────────────────────────────────────────────────────

    void registerHandlers(Spark::ModId modId) {
        Spark::SpawnObject::addHandler(modId, +[](void* /*ctx*/, auto next,
            Engine::SpawnObjectArgs* options, uint32_t flags, uint64_t param_3, uint64_t param_4) -> uint32_t
        {
            uint32_t projectileHandle = next(options, flags, param_3, param_4);

            // Only care about projectiles fired by the local player.
            if (options->ownerEntityHandle != Engine::getPlayerHandle())
                return projectileHandle;

            // Check the held weapon's kick config.
            uint32_t weaponHandle      = Engine::getPlayerHeldWeaponHandle();
            const KickConfig* cfg      = kickConfigForWeapon(weaponHandle);
            if (!cfg) return projectileHandle;

            // Ignore projectiles that are not of the type specified in the kick config.
            auto projectileTag = Engine::getTag(options->objectTagId);
            if (cfg->projectileType != "" && std::string(projectileTag->getResourcePath()) != cfg->projectileType)
                return projectileHandle;

            // Debounce: apply at most once per game tick regardless of pellet count.
            auto* playerEnt = Engine::getPlayerEntity();
            uint32_t currentTick = playerEnt ? playerEnt->ageMilis : 0;
            if (currentTick == sLastKickTick) return projectileHandle;
            sLastKickTick = currentTick;

            applyKick(*cfg);
            return projectileHandle;
        }, nullptr);
    }

}
