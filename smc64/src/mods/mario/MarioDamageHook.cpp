#include "MarioDamageHook.hpp"
#include "MarioState.hpp"
#include "spark/hook/Hooks.hpp"
#include <string>
#include "libsm64.h"
#include "decomp/sm64.h"
#include "decomp/audio_defines.h"
#include "Coordinates.hpp"
#include "MarioShieldRegen.hpp"

namespace Mod::Mario::MarioDamageHook {

    void registerHandlers(Spark::ModId modId) {
        Spark::DamageEntity::addHandler(modId, +[](void* /*ctx*/, auto next, Engine::DamageEvent* event, uint32_t entityHandle, uint16_t p2, uint16_t p3, int16_t hitBoneIndex, uint64_t p5) {
            if (!enableMario || !possessMario) return next(event, entityHandle, p2, p3, hitBoneIndex, p5);

            auto playerHandle = Engine::getPlayerHandle();
            auto playerEntity = Engine::getPlayerEntity();
            auto damageTypeTagId = event->damageTypeTagHandle;
            auto damageTag = Engine::getTag(event->damageTypeTagHandle);
            
            std::string damageTagPath = damageTag ? damageTag->getResourcePath() : "";
            bool isExplosion = damageTagPath.find("explosion") != std::string::npos;
            
            if (!playerEntity || !damageTag) return next(event, entityHandle, p2, p3, hitBoneIndex, p5);

            auto victimEntity = Engine::getEntityPointer(entityHandle);
            if (!victimEntity) return next(event, entityHandle, p2, p3, hitBoneIndex, p5);
            auto victimTag = Engine::getTag(victimEntity->tagID);
            if (!victimTag) return next(event, entityHandle, p2, p3, hitBoneIndex, p5);
            std::string victimTagPath = victimTag->getResourcePath();

            // === Damage to the player ===
            if (entityHandle == playerHandle) {
                
                // Ignore fall and vehicle collision damage to the player when Mario is active.
                if (damageTagPath.find("globals\\falling") != std::string::npos) return;
                if (damageTagPath.find("globals\\distance") != std::string::npos) return;
                if (damageTagPath.find("globals\\vehicle_collision") != std::string::npos) return;

                bool isShell = (marioState.action & ACT_FLAG_RIDING_SHELL) != 0;
                bool isMelee = damageTagPath.find("melee") != std::string::npos;

                // Mario takes extra damage while riding the shell.
                if (isShell && !isMelee) event->damageMultiplier *= 1.5f;

                // Explosions launch mario.
                if (isExplosion) {
                    Vec3 impulse = event->hitDirection * event->baseDamage * 0.25;
                    Vec3 marioImpulse = Coordinates::haloToMario(impulse);

                    bool isSelfDamage = (event->attackerHandle == playerHandle);
                    bool wasLongJumping = marioState.action == ACT_LONG_JUMP; 

                    uint32_t action;
                    if (!isSelfDamage) {
                        action = ACT_THROWN_BACKWARD;
                        sm64_set_mario_action(marioId, ACT_THROWN_BACKWARD);
                    } else if (!wasLongJumping) {
                        // sm64_set_mario_action(marioId, ACT_DIVE);
                        // sm64_set_mario_action(marioId, ACT_VERTICAL_WIND);
                        sm64_set_mario_action(marioId, ACT_SHOT_FROM_CANNON);
                    } else {
                        impulse.x *= 10.0f;
                        impulse.y *= 10.0f;
                        impulse.z *= 0.1f;
                    }

                    sm64_set_mario_velocity(marioId, 
                        marioImpulse.x + marioState.velocity[0], 
                        marioImpulse.y + marioState.velocity[1], 
                        marioImpulse.z + marioState.velocity[2]);
                    
                    // Mario doesn't take explosion damage from himself.
                    if (isSelfDamage) {
                        return;
                    }
                }

                bool isOvercharge = (damageTagPath.find("weapons\\plasma rifle\\charged bolt") != std::string::npos);
                if (isOvercharge) sm64_set_mario_action(marioId, ACT_BURNING_GROUND);
            }

            // === Damage from the player ===
            auto attackerHandle = event->attackerHandle;
            if (attackerHandle == playerHandle) {
                // If the damage is dealt by the player and is melee, ignore it.
                auto damageTypeTagId = event->damageTypeTagHandle;
                auto damageTag = Engine::getTag(damageTypeTagId);
                std::string path = damageTag ? damageTag->getResourcePath() : "";
                if (path.find("melee") != std::string::npos) return;
                
                bool victimIsFlood = victimTagPath.find("flood") != std::string::npos;

                // If the player kills an enemy mid-air, he regenerates shield.
                if (marioAirborne() && !isExplosion && !victimIsFlood) {
                    auto entity = Engine::getEntityPointer(entityHandle);
                    auto playerEntity = Engine::getPlayerEntity();
                    if (entity && playerEntity) {
                        bool wasAlive = entity->health > 0;
                        next(event, entityHandle, p2, p3, hitBoneIndex, p5);
                        bool dead = entity->health <= 0;
                        if (wasAlive && dead) {
                            regenerateShield(playerEntity, 1.0f, false);

                            sm64_play_sound_global(SOUND_GENERAL_COLLECT_1UP);
                        }
                        return;
                    }
                }

                // sm64_play_sound_global(SOUND_OBJ_STOMPED);
                // sm64_play_sound_global(SOUND_OBJ_ENEMY_DEATH_HIGH);
            }
            
            next(event, entityHandle, p2, p3, hitBoneIndex, p5);

        }, nullptr);
    }

}
