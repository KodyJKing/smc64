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

    void processEntity(uint32_t entityHandle, Halo1::Entity* entity) {
        if (!entity) return;
        if (isMario(entity)) {
            updatePose(entityHandle, entity);
        }
    }
}
