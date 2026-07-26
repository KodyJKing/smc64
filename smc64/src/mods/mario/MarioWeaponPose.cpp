#include "MarioWeaponPose.hpp"
#include "MarioWeaponOffset.hpp"
#include "MarioAimingIK.hpp"
#include "MarioSkeleton.hpp"
#include "spark/hook/Hooks.hpp"
#include "engine/halo1.hpp"
#include "math/Math.hpp"

namespace Mod::Mario::MarioWeaponPose {

    static Engine::Transform lerpTransforms(Engine::Transform& a, Engine::Transform& b, float t) {
        Engine::Transform result;
        result.pos = a.pos * (1.0f - t) + b.pos * t;
        result.x = a.x * (1.0f - t) + b.x * t;
        result.y = a.y * (1.0f - t) + b.y * t;
        result.z = a.z * (1.0f - t) + b.z * t;
        result.w = a.w * (1.0f - t) + b.w * t;
        return result;
    }

    static Engine::Transform slerpTransforms(Engine::Transform& a, Engine::Transform& b, float t) {
        Engine::Transform result = lerpTransforms(a, b, t);
        Engine::orthonormalize(result);
        return result;
    }

    static void updateWeaponPose(uint32_t weaponHandle) {
        auto rec = Engine::getEntityRecord(weaponHandle);
        if (!rec) return;
        auto weaponEntity = rec->entity();
        if (!weaponEntity) return;

        auto weaponBones = weaponEntity->worldBones.get(weaponEntity, 0);
        if (!weaponBones) return;

        auto leftHandBone = getMarioBoneByName("frame left_hand");

        auto weaponRootBone = &weaponBones[0];
        auto rootBoneInitial = weaponRootBone[0];

        MarioWeaponOffset::Offset off;
        MarioWeaponOffset::getWeaponOffset(weaponHandle, off);
        weaponRootBone->pos = leftHandBone.transformPoint(off);

        {
            float t = Mod::Mario::MarioAimingIK::armsBusySmoothed();

            Engine::Transform matchHandTransform = {};
            matchHandTransform.pos = leftHandBone.transformPoint(off);
            matchHandTransform.x = leftHandBone.x;
            // Rotate 180 about x axis to account for difference in hand and weapon coordinate systems.
            matchHandTransform.y = leftHandBone.y * -1.0f;
            matchHandTransform.z = leftHandBone.z * -1.0f;
            matchHandTransform.w = 1.0f;

            Engine::Transform currentTransform = *weaponRootBone;
            
            Engine::Transform blendedTransform = slerpTransforms(currentTransform, matchHandTransform, t);

            *weaponRootBone = blendedTransform;
        }

        auto initialInverse = Engine::inverseTransform(rootBoneInitial);
        auto relativeTransform = Engine::multiplyTransforms(
            *weaponRootBone,
            initialInverse
        );
        auto boneCount = weaponEntity->worldBones.count();
        for (int i = 1; i < boneCount; i++) {
            auto& bone = weaponBones[i];
            bone = Engine::multiplyTransforms(relativeTransform, bone);
        }
    }

    void addHandlers(Spark::ModId modId) {
        Spark::UpdateWorldBones::addHandler(modId, +[](void*, auto next, uint32_t entityHandle) {
            next(entityHandle);

            if (entityHandle == Engine::getPlayerHeldWeaponHandle()) {
                updateWeaponPose(entityHandle);
            }
        }, nullptr);

        Spark::SpawnObject::addHandler(modId, +[](void*, auto next, Engine::SpawnObjectArgs* spawnArgs, uint32_t flags, uint64_t param_3, uint64_t param_4) -> uint32_t {
            auto result = next(spawnArgs, flags, param_3, param_4);

            // If the object spawned was a projectile owned by the player, jump to an arms unbusy state without delay.
            bool isPlayers = spawnArgs->ownerEntityHandle == Engine::getPlayerHandle();
            if (isPlayers) MarioAimingIK::unbusyArms();

            return result;
        }, nullptr);
    }

}
