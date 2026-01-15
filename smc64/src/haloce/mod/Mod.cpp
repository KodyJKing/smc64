#include "Mod.hpp"
#include <Windows.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <unordered_set>
#include "MinHook.h"
#include "utils/Utils.hpp"
#include "utils/UnloadLock.hpp"
#include "math/Math.hpp"
#include "asm/AsmHelper.hpp"
#include "memory/Memory.hpp"
#include "../halo1/halo1.hpp"
#include "DllMain.hpp"
#include "Mario.hpp"
#include "modules/freecam.hpp"

namespace HaloCE::Mod {

    uintptr_t halo1 = 0;

    //////////////////////////////////////////////////////////////////
    // Animation state

    // Data associated with entity for pose interpolation.
    struct AnimationState {
        uint16_t animId;
        uint16_t frame;
        float frameProgress = 0;
        uint64_t lastUpdateTick = 0; 
    };
    // Bones pointer to animation state.
    std::unordered_map<uint64_t, AnimationState> animationStates;
    
    uint64_t tickCount = 0;

    void clearStaleAnimationStates() {
        std::vector<uint64_t> staleStates;
        for (auto& [bonesPtr, state] : animationStates) {
            if (tickCount - state.lastUpdateTick > 10)
                staleStates.push_back( bonesPtr );
        }
        for (auto bonesPtr : staleStates) {
            // std::cout << "Clearing stale animation state for bones at " << (void*) bonesPtr << std::endl;
            animationStates.erase( bonesPtr );
        }
        // if (tickCount % 120 == 0)
        //     std::cout << "Animation states: " << animationStates.size() << std::endl;
    }

    AnimationState* getAnimationState( Halo1::Entity* entity ) {
        auto bones = entity->getBoneTransforms();
        if (!bones) return nullptr;
        auto bonesPtr = (uint64_t) bones;
        if ( !animationStates.count( bonesPtr ) )
            animationStates[bonesPtr] = AnimationState{};
        return &animationStates[bonesPtr];
    }

    //////////////////////////////////////////////////////////////////
    // Hooks
    
    typedef void (*updateAllEntities_t)( void );
    updateAllEntities_t originalUpdateAllEntities = nullptr;
    //
    void hkUpdateAllEntities() {
        UnloadLock lock; // No unloading while we're still executing hook code.

        Freecam::update();
        Mario::update();

        originalUpdateAllEntities();
        clearStaleAnimationStates();
        tickCount++;
    }
    
    typedef uint64_t (*updateEntity_t)( uint32_t entityHandle );
    updateEntity_t originalUpdateEntity = nullptr;
    //
    uint64_t hkUpdateEntity( uint32_t entityHandle ) {
        UnloadLock lock; // No unloading while we're still executing hook code.

        auto rec = Halo1::getEntityRecord( entityHandle );
        if (!rec) 
            return originalUpdateEntity( entityHandle);
            
        auto entity = rec->entity();
        if (!entity)  
            return originalUpdateEntity( entityHandle );

        // Do update
        uint64_t result = originalUpdateEntity( entityHandle );

        // ...

        return result;
    }

    /*
        This function seems to only update relative bone transforms from a static animation.
        It doesn't seem to handle procedural animations like IK. It also doesn't seem to update world matrices for bones.
        It looks like the function that calls this is responsible for updating world matrices.
    */
    typedef void (*animateBones_t)(uint64_t param1, void* animation, uint16_t frame, Halo1::Transform* bones);
    animateBones_t originalAnimateBones = nullptr;
    //
    void hkAnimateBones(uint64_t param1, void* animation, uint16_t frame, Halo1::Transform* bones) {
        UnloadLock lock; // No unloading while we're still executing hook code.

        if (
            !settings.enableTimeScale || 
            !settings.poseInterpolation ||
            !animationStates.count((uint64_t) bones)
        )
            return originalAnimateBones(param1, animation, frame, bones);

        auto animState = animationStates[(uint64_t) bones];
        if (animState.frame == frame)
            return originalAnimateBones(param1, animation, frame, bones);

        // Todo: Interpolate poses in transitions between different animations, not just frames.

        // Get initial bone state.
        originalAnimateBones(param1, animation, animState.frame, bones);

        // Save a snapshot.
        uint16_t boneCount = Halo1::boneCount(animation);
        std::vector<Halo1::Transform> boneSnap;
        for (uint16_t i = 0; i < boneCount; i++)
            boneSnap.push_back(bones[i]);

        // Get target bone state.
        originalAnimateBones(param1, animation, frame, bones);

        // Interpolate between snapshot and target pose by frameProgress.
        float progress = animState.frameProgress;
        for ( uint16_t i = 0; i < boneCount; i++ ) {
            auto& bone = bones[i];
            auto& snap = boneSnap[i];
            bone.rotation = snap.rotation.nlerp(bone.rotation, progress);
            bone.translation = snap.translation.lerp(bone.translation, progress);
            bone.scale = snap.scale + (bone.scale - snap.scale) * progress;
        }
    }

    typedef void (*damageEntity_t)(uint32_t entityHandle, uint16_t param_2, uint16_t param_3, uint64_t param_4, uint8_t* param_5, void* param_6, uint64_t param_7, uint64_t param_8, uint32_t* param_9, float* param_10, uint32_t* param_11, float damage, char param_13);
    damageEntity_t originalDamageEntity = nullptr;
    //
    void hkDamageEntity(uint32_t entityHandle, uint16_t param_2, uint16_t param_3, uint64_t param_4, uint8_t* param_5, void* param_6, uint64_t param_7, uint64_t param_8, uint32_t* param_9, float* param_10, uint32_t* param_11, float damage, char param_13) {
        UnloadLock lock; // No unloading while we're still executing hook code.
        
        if (!settings.enableTimeScale)
            return originalDamageEntity(entityHandle, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10, param_11, damage, param_13);
        
        auto rec = Halo1::getEntityRecord( entityHandle );
        if (!rec)
            return originalDamageEntity(entityHandle, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10, param_11, damage, param_13);

        if (rec->typeId == Halo1::TypeID_Player)
            damage *= settings.playerDamageScale;
        else
            damage *= settings.npcDamageScale;


        float oldHealth = 0.0f;
        if (rec->entity())
            oldHealth = rec->entity()->health;

        originalDamageEntity(entityHandle, param_2, param_3, param_4, param_5, param_6, param_7, param_8, param_9, param_10, param_11, damage, param_13);

        float newHealth = 0.0f;
        if (rec->entity())
            newHealth = rec->entity()->health;
        
        if (newHealth <= 0.0 && oldHealth > 0.0) {
            std::cout << "Entity " << entityHandle << " died." << std::endl;
        }
    }

    typedef float(*getShieldDamageResist_t)(uint32_t entityHandle, bool useExtraScalar);
    getShieldDamageResist_t originalGetShieldDamageResist = nullptr;
    //
    float hkGetShieldDamageResist(uint32_t entityHandle, bool useExtraScalar) {
        UnloadLock lock; // No unloading while we're still executing hook code.

        auto rec = Halo1::getEntityRecord( entityHandle );
        if (!rec)
            return originalGetShieldDamageResist(entityHandle, useExtraScalar);

        float result = originalGetShieldDamageResist(entityHandle, useExtraScalar);
        if (rec->typeId == Halo1::TypeID_Player)
            result /= settings.playerDamageScale;
        else
            result /= settings.npcDamageScale;

        return result;
    }

    // updateActor updates AIs.
    typedef void (*updateActor)(uint64_t actorHandle);
    updateActor originalUpdateActor = nullptr;
    //
    void hkUpdateActor(uint64_t actorHandle) {
        UnloadLock lock; // No unloading while we're still executing hook code.
        // ...
        originalUpdateActor(actorHandle);
    }

    void hookFunctions() {
        std::cout << "\nHooking functions:\n" << std::endl;

        #define HOOK_FUNC( func, offset) \
            void* p##func = (void*) (halo1 + offset); \
            std::cout << #func << ": " << std::endl; \
            std::cout << AsmHelper::disassemble( (uint8_t*) p##func, 0x100 ) << std::endl; \
            MH_CreateHook( p##func, hk##func, (void**) &original##func ); \
            MH_EnableHook( p##func );

        // HOOK_FUNC( UpdateEntity, 0xB3A06CU );
        // HOOK_FUNC( AnimateBones, 0xC41984U );
        HOOK_FUNC( UpdateAllEntities, 0xB35654U );
        // HOOK_FUNC( DamageEntity, 0xB9FBD0U );
        // HOOK_FUNC( GetShieldDamageResist, 0xB9D114U );
        // HOOK_FUNC( UpdateActor, 0xC04A14U );

        #undef HOOK_FUNC
    }

    void unhookFunctions() {
        std::cout << "\nUnhooking functions." << std::endl;
        // MH_RemoveHook( (void*) originalUpdateEntity );
        // MH_RemoveHook( (void*) originalAnimateBones );
        MH_RemoveHook( (void*) originalUpdateAllEntities );
        // MH_RemoveHook( (void*) originalDamageEntity );
        // MH_RemoveHook( (void*) originalGetShieldDamageResist );
        // MH_RemoveHook( (void*) originalUpdateActor );
    }

    //////////////////////////////////////////////////////////////////

    std::vector<Memory::PatchPtr> patches;
    void patchTags() {
        std::string weaponsToRoFLimit[] = {
            "characters\\sentinel\\sentinel",
            "weapons\\plasma pistol\\plasma pistol",
        };

        for (const std::string& weapName : weaponsToRoFLimit) {
            auto weapTag = Halo1::findTag( weapName.c_str(), "weap" );
            std::cout << weapName << " tag: " << weapTag << std::endl;
            if (!weapTag) continue;
            auto projData = Halo1::getProjectileData( weapTag, 0 );
            std::cout << weapName << " Projectile data: " << projData << std::endl;
            if (!projData) continue;

            patches.push_back( Memory::createPatch( projData->minRateOfFire, 15.0f ) );
            patches.push_back( Memory::createPatch( projData->maxRateOfFire, 15.0f ) );
        }

        std::string hitscanProjectiles[] = {
            "weapons\\assault rifle\\bullet",
            "weapons\\pistol\\bullet",
            "weapons\\sniper rifle\\sniper bullet",
            "weapons\\shotgun\\pellet",
            "vehicles\\warthog\\bullet"
        };
        for (const std::string& projName : hitscanProjectiles) {
            auto projTag = Halo1::findTag( projName.c_str(), "proj" );
            std::cout << projName << " tag: " << projTag << std::endl;
            if (!projTag) continue;
            auto projData = (Halo1::ProjectileTagData*) projTag->getData();
            std::cout << projName << " Projectile data: " << projData << std::endl;
            if (!projData) continue;

            const float speed = 1.5f;
            patches.push_back( Memory::createPatch( projData->initialSpeed, speed ) );
            patches.push_back( Memory::createPatch( projData->finalSpeed, speed ) );
            patches.push_back( Memory::createPatch( projData->arc, 0.0f ) );
        }
    }
    void unpatchTags() {
        patches.clear();
    }

    bool isInstalled = false;

    void init() {
        if (isInstalled)
            return;
        isInstalled = true;

        const std::string moduleName = "halo1.dll";
        halo1 = (uintptr_t) Utils::waitForModule(moduleName);
        std::cout << moduleName << ": " << (void*) halo1 << std::endl;

        hookFunctions();

        Freecam::init( halo1 );
        Mario::init();

        // #define PATCH_TAGS
        #ifdef PATCH_TAGS
        patchTags();
        #endif

        std::cout << "Mod installed." << std::endl;
    }

    void free() {
        if (!isInstalled)
            return;
        isInstalled = false;

        Mario::free();
        Freecam::free();

        #ifdef PATCH_TAGS
        unpatchTags();
        #endif
        unhookFunctions();

        std::cout << "Mod uninstalled." << std::endl;
    }

    // Called by mod dll's thread regularly.
    void modThreadUpdate() {
        if (Halo1::isGameLoaded()) {
            init();
        } else if (isInstalled) {
            ModHost::reinitialize();
        }
    }

}
