#include "MarioShell.hpp"

#include "functions/MarioWorldPosition.hpp"
#include "spark/hook/Hooks.hpp"
#include "engine/halo1.hpp"
#include "MarioState.hpp"
#include "MarioSkeleton.hpp"
#include "libsm64.h"
#include "decomp/sm64.h"
#include "functions/KillPlayer.hpp"

namespace Mod::Mario::Shell {

    bool isShield(Engine::Entity* entity) {
        return entity && entity->fromResourcePath(SHIELD_RESOURCE_PATH);
    }
    bool isShield(uint32_t entityHandle) {
        auto entity = Engine::getEntityPointer(entityHandle);
        return isShield(entity);
    }

    bool isMarioInCrashState() {
        return marioState.action == ACT_BACKWARD_GROUND_KB;
    }

    bool isMarioExitingShell() {
        return marioState.action == ACT_BACKWARD_GROUND_KB || marioState.action == ACT_CROUCH_SLIDE;
    }

    bool isMarioInShellAction() {
        return marioState.action & ACT_FLAG_RIDING_SHELL;
    }

    void putMarioOnShell() {
        if (isMarioExitingShell()) return;
        if (isMarioInShellAction()) return;
        sm64_set_mario_action(marioId, ACT_RIDING_SHELL_GROUND);
    }

    void removeEquipmentEntity(Engine::Entity* entity) {
        // In liu of a propery way to do this, just teleport the entity way under the map.
        if (entity) {
            entity->pos.y = -1000.0f;
        }
    }

    bool marioIsTouchingShellEquipment() {
        auto marioPos = marioWorldPosition();
        bool touching = false;
        Engine::foreachEntityInRadius(marioPos, 0.1f, [&touching](Engine::Entity* entity) {
            if (!touching && isShield(entity)) {
                removeEquipmentEntity(entity);
                touching = true;
            }
        });
        return touching;
    }

    void checkForCrash() {
        static uint32_t wasRidingShell = false;
        if (wasRidingShell && isMarioInCrashState()) {
            // killPlayer();
            damagePlayer(20.0f, 1.0f);
        }
        wasRidingShell = isMarioInShellAction();
    }

    void updateShellState() {
        checkForCrash();
        if (isMarioInShellAction()) return;
        if (marioIsTouchingShellEquipment()) {
            putMarioOnShell();
        }
    }

    void updateShellPose() {
        auto shell = getMarioBonePointerByName("frame shield");
        if (!shell) return;

        if (!isMarioInShellAction()) {
            shell->pos = Vec3{0.0f, 0.0f, 0.0f};
            shell->w = 0.0f;
            return;
        }

        auto pelvis = getMarioBonePointerByName("frame pelvis");
        if (!pelvis) return;

        auto pos = marioWorldPosition();
        
        shell->x = pelvis->z;
        shell->y = pelvis->x;
        shell->z = pelvis->y;
        shell->pos = pos + pelvis->x * 0.05f;
        shell->w = 1.0f;
    }

}
