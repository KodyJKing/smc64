#include "../common.hpp"
#include "haloce/halo1/halo1.hpp"

namespace HaloCE::Mod::ThirdPersonFix {

    typedef uint32_t (*spawnProjectile)(Halo1::ProjectileSpawnArgs* options, uint32_t flags);
    spawnProjectile originalSpawnProjectile = nullptr;
    uint32_t hkSpawnProjectile(Halo1::ProjectileSpawnArgs* options, uint32_t flags) {
        UnloadLock lock; // No unloading while we're still executing hook code.

        if (options->ownerEntityHandle != Halo1::getPlayerHandle()) {
            return originalSpawnProjectile(options, flags);
        }

        Vec3 spawnPosition = options->spawnPosition;
        auto camera = Halo1::getPlayerCameraPointer();
        if (camera && Memory::isAllocated(camera)) {
            float depth = (options->spawnPosition - camera->pos).dot( camera->fwd.normalize() );
            spawnPosition = camera->pos + camera->fwd.normalize() * depth;
        }

        options->spawnPosition = spawnPosition;
        uint32_t projectileHandle = originalSpawnProjectile(options, flags);

        auto projectile = Halo1::getEntityPointer(projectileHandle);
        if (projectile && Memory::isAllocated(projectile)) {
            if (camera && Memory::isAllocated(camera)) {
                // Move projectile spawn position to camera position
                projectile->pos = spawnPosition;

                // Adjust projectile velocity to match camera forward vector
                Vec3 vel = projectile->vel;
                Vec3 fwd = camera->fwd.normalize();
                float speed = vel.length();
                projectile->vel = fwd * speed;

                // Todo: Reimplement spread.
            }
        }

        return projectileHandle;
    }

    void init(uintptr_t halo1) {
        // Hook projectile spawn function
        HOOK_FUNC( SpawnProjectile, 0xB35FD4U );
    }

    void free() {
        // Unhook projectile spawn function
        MH_RemoveHook( (void*) originalSpawnProjectile );
    }

}
