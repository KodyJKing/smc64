#include "MarioGoombaStomp.hpp"

#include "MarioState.hpp"
#include "MarioShieldRegen.hpp"
#include "Mario.hpp"
#include "Coordinates.hpp"
#include "engine/halo1.hpp"
#include "engine/entity/entity_list.hpp"
#include "spark/hook/Hooks.hpp"
#include "decomp/sm64.h"
#include "decomp/audio_defines.h"
#include "libsm64.h"

#include <unordered_map>
#include <cmath>
#include <string>

#define DEBUG_MARIO_GOOMBA_STOMP 1

#ifdef DEBUG_MARIO_GOOMBA_STOMP
    #include <iostream>
    #include <fstream>
    #include <ctime>
    #include <sstream>
    #include <iomanip>
    #define LOG(x) std::cout << "[MarioGoombaStomp] " << x << std::endl;
#else
    #define LOG(x) ;
#endif

namespace Mod::Mario::GoombaStomp {

    // ── Tunable constants ──────────────────────────────────────────────────────
    static constexpr uint32_t kStompAnimTicks   = 15;       // ticks for squash animation (~0.5s at 30 fps)
    static constexpr float    kStompDamage      = 0.5f;    // damage fraction per stomp
    static constexpr float    kStompBounceY     = 30.0f;   // upward velocity in Mario units after a stomp
    static constexpr float    kStompWindowAbove = 0.25f;    // Halo units: window above enemy top where stomp still triggers
    static constexpr float    kSquashMin        = 0.01f;    // minimum vertical scale at stomp moment
    static constexpr float    kStompShieldRegenAmount = 0.2f; // fraction of shield regenerated per stomp
    static constexpr float    kGroundPoundRadiusBoost = 0.25f; // additional horizontal radius when ground-pounding

    struct StompEventType {
        uint32_t action; // The action to set Mario to after a stomp
        int32_t soundEffect; // The sound effect to play when a stomp occurs
        float bounceY; // The upward velocity in Mario units after a stomp
        float damage; // The damage fraction inflicted by the stomp
        bool noAction;
        bool noBounce;
        std::string dropEquipment = ""; // The equipment entity to drop when a stomp occurs.
    };

    StompEventType groundPound = { .soundEffect = SOUND_OBJ_STOMPED, .damage = kStompDamage, .noAction = true, .noBounce = true, };
    StompEventType regularStomp = { .action = ACT_FREEFALL, .soundEffect = SOUND_OBJ_STOMPED, .bounceY = kStompBounceY, .damage = kStompDamage };
    StompEventType twirlStomp = { .action = ACT_TWIRLING, .soundEffect = SOUND_MARIO_TWIRL_BOUNCE, .bounceY = kStompBounceY * 5.0f, .damage = kStompDamage };
    StompEventType friendlyStomp = { .action = ACT_TRIPLE_JUMP, .soundEffect = SOUND_OBJ_STOMPED, .bounceY = kStompBounceY * 3.0f, .damage = 0.0f };
    StompEventType jackalStomp = { .action = ACT_FREEFALL, .soundEffect = SOUND_OBJ_STOMPED, .bounceY = kStompBounceY, .damage = kStompDamage, .dropEquipment = "smc64\\jackal_shield\\jackal_shield" };

    static std::unordered_map<std::string, StompEventType> postStompAction = {
        {"characters\\sentinel\\sentinel", twirlStomp},
        {"characters\\floodcarrier\\floodcarrier", twirlStomp},
        {"characters\\marine_armored\\marine_armored", friendlyStomp},
        {"characters\\marine\\marine_plasma_rifle", friendlyStomp},
        {"characters\\captain_ingame\\captain_ingame", friendlyStomp},
        {"characters\\jackal\\jackal", jackalStomp},
        {"characters\\jackal\\jackal major", jackalStomp},
    };

    StompEventType getStompEventType(const std::string& entityPath) {
        // Check if there is a specific post-stomp action for this entity.
        if (postStompAction.count(entityPath)) {
            return postStompAction[entityPath];
        }
        if (marioState.action == ACT_GROUND_POUND) {
            return groundPound;
        }
        return regularStomp;
    }

    struct StompEvent {
        uint32_t animationTimer; // Counts down to 0.
        StompEventType type; // The type of stomp event (regular, ground pound, twirl, etc.)
    };

    // Maps entity handles to their corresponding stomp events.
    static std::unordered_map<uint32_t, StompEvent> stompEvents;

    // ── Helpers ────────────────────────────────────────────────────────────────

    static float shoulderHeight(Engine::Entity* entity) {
        if (!entity) return 999.9f;
        if (entity->fromResourcePath("characters\\flood_infection\\flood_infection"))
            return -0.125f;
        return 0.25f;
    }

    static float horizontalRadius(Engine::Entity* entity) {
        return 0.3f;
    }

    static float horizontalBoost() {
        if (marioState.action == ACT_GROUND_POUND) {
            return kGroundPoundRadiusBoost; // boost horizontal radius when ground-pounding
        }
        return 0.0f;
    }

    static bool canStompInAction(uint32_t action) {
        // Only allow stomps in freefall or jump actions.
        switch (action) {
            case ACT_FREEFALL:
            case ACT_JUMP:
            case ACT_DOUBLE_JUMP:
            case ACT_TRIPLE_JUMP:
            case ACT_LONG_JUMP:
            case ACT_WALL_KICK_AIR:
            case ACT_SIDE_FLIP:
            case ACT_GROUND_POUND:
            case ACT_BACKFLIP:
            case ACT_FORWARD_ROLLOUT:
            case ACT_LAVA_BOOST:
                return true;
            default:
                return false;
        }
    }

    static bool canStomp(Engine::Entity* entity, uint32_t entityHandle) {
        if (!canStompInAction(marioState.action)) return false;
        if (entity->entityCategory != Engine::EntityCategory_Biped) return false;
        if (entity->health <= 0.0f) return false;
        if (entityHandle == Engine::getPlayerHandle()) return false;
        // Don't re-stomp while the squash animation is still playing.
        if (stompEvents.count(entityHandle)) return false;
        return true;
    }

    static bool shouldBounce() {
        return marioState.action != ACT_GROUND_POUND;
    }

    static Engine::Tag* getDamageTag() {
        return Engine::findTag("weapons\\frag grenade\\explosion", "jpt!");
    }

    static void spawnEquipment(const std::string& resourcePath, Vec3 position, uint32_t victimHandle) {
        if (resourcePath.empty()) return;

        auto resourcePathCstr = resourcePath.c_str();
        auto tag = Engine::findTag(resourcePathCstr, "eqip");
        if (!tag) return;

        Engine::SpawnObjectArgs args{0};
        args.objectTagId = tag->tagID;
        args.spawnPosition = position;
        // args.ownerEntityHandle = victimHandle;
        args.ownerEntityHandle = NULL_HANDLE;
        
        args.unknown1 = 0;
        args.unknown2 = 0;
        args.unknown3 = 0xFFFFFFFF;
        args.unknown4 = 0;
        args.unknown5 = 3;

        uint32_t flags = 3;

        auto handle = Spark::SpawnObject::dispatch(&args, flags, 0, 0);

        auto entity = Engine::getEntityPointer(handle);
        if (!entity) {
            Beep(1440, 50);
            Beep(1440, 50);
            return;
        }

        entity->pos = position + Vec3{ 0.0f, 0.0f, 0.1f };
        entity->vel = Vec3{ 0.0f, 0.0f, 0.0f };
        entity->angularVelocity = Vec3{ 0.0f, 0.0f, 0.001f };
    }

    static void dealStompDamage(uint32_t entityHandle, Vec3 hitPos, StompEvent& stompEvent) { 
        if (!Spark::DamageEntity::original) return;
        auto* tag = getDamageTag();
        if (!tag) return;

        float amount = stompEvent.type.damage;
        float multiplier = amount > 0.0f ? 1.0f : 0.0f;

        Engine::DamageEvent ev{};
        ev.damageTypeTagHandle = tag->tagID;
        ev.flags               = 0x1; // single-entity target
        ev.interactorHandle    = NULL_HANDLE;
        ev.attackerHandle      = Engine::getPlayerHandle();
        ev.sourceTypeIndex     = (uint16_t)-1;
        ev.hitPosition         = hitPos;
        ev.hitDirection        = Vec3{ 0.0f, 0.0f, -1.0f }; // downward
        ev.baseDamage          = amount;
        ev.damageMultiplier    = multiplier;
        Spark::DamageEntity::dispatch(&ev, entityHandle, 0, 0, -1, 0);

        bool fatal = false;
        if (auto* entity = Engine::getEntityPointer(entityHandle)) {
            fatal = entity->health <= 0.0f;
        }

        if (fatal) {
            // Play a sound effect for the stomp kill.
            sm64_play_sound_global(SOUND_OBJ_ENEMY_DEATH_HIGH);

            // Spawn equipment for the stomp kill.
            spawnEquipment(stompEvent.type.dropEquipment, hitPos, entityHandle);
        }
    }

    // ── Stomp detection ────────────────────────────────────────────────────────

    static void checkForStomps() {
        if (!enableMario || !possessMario || marioId < 0) return;

        // Mario must be moving downward (Mario Y == Halo Z, i.e. vertical).
        if (marioState.velocity[1] >= 0.0f) return;

        Vec3 marioHaloPos = marioWorldPosition();

        Engine::foreachEntityRecordIndexed([&](Engine::EntityRecord* rec, uint16_t index) {
            uint32_t handle = ((uint32_t)rec->id << 16) | index;

            auto* entity = rec->entity();
            if (!entity) return;
            if (!canStomp(entity, handle)) return;

            float topZ    = entity->pos.z + shoulderHeight(entity);
            float hRadius = horizontalRadius(entity);

            // Mario must be somewhere between the entity base and just above its top.
            if (marioHaloPos.z < entity->pos.z) return;
            if (marioHaloPos.z > topZ + kStompWindowAbove) return;

            // Check horizontal proximity (Halo XY plane).
            float dx = marioHaloPos.x - entity->pos.x;
            float dy = marioHaloPos.y - entity->pos.y;
            float hBoost = horizontalBoost();
            hRadius += hBoost;
            if (dx * dx + dy * dy > hRadius * hRadius) return;

            // ── Stomp detected! ────────────────────────────────────────────────
            auto stompType = getStompEventType(entity-> getTagResourcePath());
            stompEvents[handle] = { kStompAnimTicks, stompType };
            
            auto player = Engine::getPlayerEntity();
            regenerateShield(player, kStompShieldRegenAmount, false);

            if (!stompType.noAction) {
                sm64_set_mario_action(marioId, stompType.action);
                sm64_set_mario_anim_frame(marioId, 0);
            }
            if (!stompType.noBounce) {
                sm64_set_mario_velocity(marioId,
                    marioState.velocity[0],
                    stompType.bounceY,
                    marioState.velocity[2]);
            }
            sm64_play_sound_global(stompType.soundEffect);
            
        });
    }

    // ── Squash animation ───────────────────────────────────────────────────────

    static void applySquashToBones(uint32_t entityHandle) {
        auto it = stompEvents.find(entityHandle);
        if (it == stompEvents.end()) return;

        auto& ev = it->second;

        if (ev.animationTimer == 0) {
            dealStompDamage(entityHandle, Engine::getEntityPointer(entityHandle)->pos, ev);
            stompEvents.erase(it);
            return;
        }

        auto* entity = Engine::getEntityPointer(entityHandle);
        if (!entity) {
            stompEvents.erase(it);
            return;
        }

        auto boneCount = entity->worldBones.count();
        auto* bones    = entity->worldBones.get(entity, 0);
        if (boneCount == 0 || !bones) {
            ev.animationTimer--;
            return;
        }

        float t     = 1.0f - (float)ev.animationTimer / (float)kStompAnimTicks;
        float scale = kSquashMin + (1.0f - kSquashMin) * t;
        // float scale = 1.0f * (1.0f - t) + kSquashMin * t; // linear interpolation from 1.0 to kSquashMin
        // float scale = kSquashMin;

        const float minScale = 0.1f;
        if (scale < minScale) scale = minScale;

        float hScale = 1 / scale;
        float hScaleRoot = sqrtf(hScale);

        float entityZ = entity->pos.z;
        for (size_t i = 0; i < boneCount; i++) {
            auto& bone = bones[i];
            float dz   = bone.pos.z - entityZ;
            bone.pos.z = entityZ + dz * scale;
            bone.x.z  *= scale;
            bone.y.z  *= scale;
            bone.z.z  *= scale;

            float dx = bone.pos.x - entity->pos.x; 
            bone.pos.x = entity->pos.x + dx * hScaleRoot;
            bone.x.x  *= hScaleRoot;
            bone.y.x  *= hScaleRoot;
            bone.z.x  *= hScaleRoot;

            float dy = bone.pos.y - entity->pos.y; 
            bone.pos.y = entity->pos.y + dy * hScaleRoot;
            bone.x.y  *= hScaleRoot;
            bone.y.y  *= hScaleRoot;
            bone.z.y  *= hScaleRoot;

            // bone.w *= hScaleRoot;
        }

        ev.animationTimer--;
    }

    // ── Init ───────────────────────────────────────────────────────────────────

    /**
     * Hooks UpdateAllEntities to handle checking for stomp conditions.
     * Hooks UpdateWorldBones to animate vertical squash effect.
     */
    void init(Spark::ModId modId) {
        Spark::UpdateAllEntities::addHandler(modId, +[](void*, auto next) {
            next();
            checkForStomps();
        }, nullptr);

        Spark::UpdateWorldBones::addHandler(modId, +[](void*, auto next, uint32_t entityHandle) {
            next(entityHandle);
            applySquashToBones(entityHandle);
        }, nullptr);
    }

}
