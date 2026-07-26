#include "Mario.hpp"
#include "MarioModel.hpp"
#include "MarioSkeleton.hpp"
#include <string>
#include "decomp/sm64.h"
#include "Coordinates.hpp"
#include "spark/hook/Hooks.hpp"
#include "functions/CheifToMario.hpp"

// #define DEBUG_MARIO_MODEL 1

#ifdef DEBUG_MARIO_MODEL
#include <iostream>
#define LOG(x) std::cout << x << std::endl;
#else
#define LOG(x)
#endif

namespace Mod::Mario::MarioModel {
    const char* marioTagPath = "smc64\\mario\\mario";

    bool isMario(Engine::Entity* entity) {
        if (!entity) return false;
        return entity->fromResourcePath(marioTagPath);
    }

    bool isMario(uint32_t entityHandle) {
        auto rec = Engine::getEntityRecord( entityHandle );
        if (!rec) return false;
        auto entity = rec->entity();
        if (!entity) return false;
        return isMario(entity);
    }

    void updateMarioPosition(Engine::Entity* marioEntity, Vec3 newPos) {
        if (!marioEntity) return;

        marioEntity->pos = newPos;
        
        // Filthy hack to work around the fact that we're bypassing Halo's entity movement bookkeeping.
        // I suspect this is updating things like cluster and wake/sleep state.
        Engine::Scripting::submit("(objects_attach (player0) \"\" mario_model \"\")");
        Engine::Scripting::submit("(objects_detach (player0) mario_model)");
    }

    void updatePose( uint32_t entityHandle, Engine::Entity* marioEntity) {
        if (!marioEntity) return;

        // For now, just set all world bone transforms to identity rotation. Keep translation and scale.
        auto worldBones = marioEntity->worldBones.get(marioEntity, 0);
        if (!worldBones) return;

        updateMarioPosition(marioEntity, marioPose[0].pos);
        Vec3 marioVelocity = *(Vec3*)&marioState.velocity[0];
        marioEntity->vel = Coordinates::marioToHalo(marioVelocity);

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

    uint32_t marioHandle  = 0xFFFFFFFF;

    uint32_t spawnMarioModel(Vec3 position) {
        auto tag = Engine::findTag(marioTagPath, "weap");
        if (!tag) return NULL_HANDLE;

        Engine::SpawnObjectArgs args{};
        args.objectTagId    = tag->tagID;
        args.ownerEntityHandle  = NULL_HANDLE;
        args.spawnPosition      = position;

        if (!Spark::SpawnObject::original) return NULL_HANDLE;
        return Spark::SpawnObject::original(&args, 3, 0, 0);
    }

    void renderEntity(Engine::RenderEntityRequest *request, Engine::renderEntity_t renderEntityOriginal) {
        // if (!isMario(request->entityHandle)) return;

        auto renderedHandle = request->entityHandle;
        auto playerHandle = Engine::getPlayerHandle();
        if (renderedHandle != playerHandle) return;
        
        uint32_t weaponHandle = Engine::getPlayerHeldWeaponHandle();
        if (weaponHandle != NULL_HANDLE && Engine::entityValid(weaponHandle)) {
            Engine::RenderEntityRequest childRequest = *request;
            childRequest.entityHandle = weaponHandle;
            renderEntityOriginal(&childRequest);
        }

        if (marioHandle != NULL_HANDLE && Engine::entityValid(marioHandle)) {
            Engine::RenderEntityRequest marioRequest = *request;
            marioRequest.entityHandle = marioHandle;
            renderEntityOriginal(&marioRequest);
        }
    }

    uint64_t marioModelLastUpdatedTick = 0;

    void addHandlers(Spark::ModId modId) {

        Spark::UpdateWorldBones::addHandler(modId, +[](void*, auto next, uint32_t entityHandle) {
            auto rec = Engine::getEntityRecord(entityHandle);
            if (!rec) return next(entityHandle);
            auto entity = rec->entity();
            if (!entity) return next(entityHandle);

            // ! Fix for camera jitter when Mario is somewhere Cheif cannot be.
            // Note: May be cleaner to hook entity collision and disable it for Chief when Mario is in control.
            auto playerHandle = Engine::getPlayerHandle();
            if (entityHandle == playerHandle && Mario::marioInControl()) {
                // Correct for Chief being pushed out of place by collision.
                cheifToMario(entity);
            }

            next(entityHandle);

            if (isMario(entity)) {
                marioModelLastUpdatedTick = GetTickCount64();
                marioHandle = entityHandle;
                updatePose(entityHandle, entity);
            }
            
        }, nullptr);

        Spark::UpdateAllEntities::addHandler(modId, +[](void*, auto next) {
            next();
            auto now = GetTickCount64();
            if (marioModelLastUpdatedTick != 0 && now - marioModelLastUpdatedTick > 50) {
                Engine::Scripting::submit("(object_create_anew mario_model)");
            }
        }, nullptr);
    }

}
