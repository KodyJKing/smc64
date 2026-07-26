#include "engine/halo1.hpp"
#include "spark/hook/Hooks.hpp"
#include "MarioSkeleton.hpp"
#include "MarioState.hpp"

#include <unordered_map>
namespace Mod::Mario::ThirdPersonFix {

    std::unordered_map<std::string, float> spreadValues = {
        {"weapons\\shotgun\\pellet", 4.0f},
        {"weapons\\assault rifle\\bullet", 0.6f},
    };

    void registerHandlers(Spark::ModId modId) {
        // Suppress flares on player-held weapons (they render in the wrong place in third-person view)
        Spark::UpdateFlareTransform::addHandler(modId, +[](void* /*ctx*/, auto next, uint32_t flareHandle) {
            auto entry = Engine::getFlareEntry(flareHandle);
            if (entry && entry->mountEntityHandle != NULL_HANDLE) {
                if (entry->mountEntityHandle == Engine::getPlayerHeldWeaponHandle()) {
                    return;
                }
            }
            next(flareHandle);
        }, nullptr);

        Spark::SpawnObject::addHandler(modId, +[](void* /*ctx*/, auto next, Engine::SpawnObjectArgs* options, uint32_t flags, uint64_t param_3, uint64_t param_4) -> uint32_t {
            auto playerHandle = Engine::getPlayerHandle();
            if (playerHandle == NULL_HANDLE) return next(options, flags, param_3, param_4);
            if (options->ownerEntityHandle != playerHandle) return next(options, flags, param_3, param_4);
            if (!Mario::marioInControl()) return next(options, flags, param_3, param_4);

            // Correct spawn position.
            Vec3 spawnPosition = options->spawnPosition;
            auto camera = Engine::getPlayerCameraPointer();
            if (camera && Memory::isAllocated(camera)) {
                
                // Reference point for choosing depth along look axis.
                Vec3 depthReference = options->spawnPosition;
                auto leftHand = Mario::getMarioBonePointerByName("left_hand");
                if (leftHand && Memory::isAllocated(leftHand)) {
                    depthReference = leftHand->pos;
                }

                
                float depth = (depthReference - camera->pos).dot(camera->fwd.normalize());
                spawnPosition = camera->pos + camera->fwd.normalize() * depth;
            }

            options->spawnPosition = spawnPosition;
            uint32_t projectileHandle = next(options, flags, param_3, param_4);

            // Correct direction.
            auto projectile = Engine::getEntityPointer(projectileHandle);
            if (projectile && Memory::isAllocated(projectile)) {
                if (camera && Memory::isAllocated(camera)) {
                    projectile->pos = spawnPosition;

                    Vec3 fwd = camera->fwd.normalize();

                    auto projectileTag = projectile->tag();
                    if (projectileTag) {
                        auto resourcePath = projectileTag->getResourcePath();
                        if (resourcePath && spreadValues.contains(resourcePath)) {
                            float spread = spreadValues[resourcePath];
                            Vec3 random = Vec3::randomGaussian(spread * 0.01f);
                            fwd = (fwd + random).normalize();
                        }
                    }

                    float speed = projectile->vel.length();
                    projectile->vel = fwd * speed;
                }
            }

            return projectileHandle;
        }, nullptr);
    }

}
