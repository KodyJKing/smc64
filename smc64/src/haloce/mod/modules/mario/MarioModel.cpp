#include "MarioModel.hpp"
#include "MarioSkeleton.hpp" 
#include <string>

namespace {
    void updateEntityRegion(uint32_t entityHandle, void* unknown) {
        typedef uint64_t (*updateEntityRegion_t)( uint32_t entityHandle, void* unknown );
        auto halo1Dll = Halo1::dllBase();
        auto pUpdateEntityRegion = (updateEntityRegion_t) (halo1Dll + 0xB368B0U);
        pUpdateEntityRegion(entityHandle, unknown);
    }
}

namespace HaloCE::Mod::Mario::MarioModel {
    const char* marioTagPath = "smc64\\mario\\mario";

    bool isMario(Halo1::Entity* entity) {
        if (!entity) return false;
        return entity->fromResourcePath(marioTagPath);
    }

    bool isMario(uint32_t entityHandle) {
        auto rec = Halo1::getEntityRecord( entityHandle );
        if (!rec) return false;
        auto entity = rec->entity();
        if (!entity) return false;
        return isMario(entity);
    }

    uint32_t playerWeaponHandle() {
        auto playerEntity = Halo1::getPlayerEntity();
        if (!playerEntity) return 0xFFFFFFFF;
        return playerEntity->childHandle;
    }

    void updatePose( uint32_t entityHandle, Halo1::Entity* marioEntity) {
        if (!marioEntity) return;

        // For now, just set all world bone transforms to identity rotation. Keep translation and scale.
        auto worldBones = marioEntity->worldBones.get(marioEntity, 0);
        if (!worldBones) return;

        // Move entity to bone0's position and update region to prevent culling issues.
        marioEntity->pos = marioPose[0].pos;
        updateEntityRegion(entityHandle, nullptr);

        auto boneCount = marioEntity->worldBones.count();
        for (int i = 0; i < boneCount; i++) {
            auto& bone = worldBones[i];
            bone.w = marioPose[i].w;
            bone.x = marioPose[i].x;
            bone.y = marioPose[i].y;
            bone.z = marioPose[i].z;
            bone.pos = marioPose[i].pos;
        }
    }

    void updateWeaponPose( uint32_t weaponHandle ) {
        // This is a test, just place all weapon bones at mario's root bone position.
        auto rec = Halo1::getEntityRecord( weaponHandle );
        if (!rec) return;
        auto entity = rec->entity();
        if (!entity) return;

        auto worldBones = entity->worldBones.get(entity, 0);
        if (!worldBones) return;

        auto leftHandBone = getMarioBoneByName("left_hand");


        auto boneCount = entity->worldBones.count();
        for (int i = 0; i < boneCount; i++) {
            worldBones[i].pos = leftHandBone.pos + leftHandBone.x * 0.05f;
            worldBones[i].x = leftHandBone.x;
            worldBones[i].y = leftHandBone.y;
            worldBones[i].z = leftHandBone.z;
            worldBones[i].w = 0.75f;
        }
    }

    void processEntity(uint32_t entityHandle, Halo1::Entity* entity) {
        if (!entity) return;
        if (isMario(entity)) {
            updatePose(entityHandle, entity);
        }
        if (entityHandle == playerWeaponHandle()) {
            updateWeaponPose(entityHandle);
        }
    }

    void renderEntity(Halo1::RenderEntityRequest *request, Halo1::renderEntity_t renderEntityOriginal) {
        if (!isMario(request->entityHandle)) return;
        
        uint32_t weaponHandle = playerWeaponHandle();
        if (weaponHandle != 0xFFFFFFFF) {
            Halo1::RenderEntityRequest childRequest = *request;
            childRequest.entityHandle = weaponHandle;
            renderEntityOriginal(&childRequest);
        }
    }
}
