#include "MarioShieldRegen.hpp"

#include "MarioState.hpp"
#include "decomp/sm64.h"
#include "decomp/audio_defines.h"
#include "libsm64.h"

namespace Mod::Mario {

    static uint32_t prevAction = 0;

    // Shield restored on the first frame of each acrobatic action.
    static constexpr float SIDE_FLIP_REGEN   = 0.2f;
    static constexpr float WALL_KICK_REGEN   = 0.3f;
    static constexpr float TRIPLE_JUMP_REGEN = 0.5f;
    
    void regenerateShield(Engine::Entity* player, float amount, bool _allowOvershield) {
        if (!player) return;
        
        float oldShield = player->shield;
        
        player->shield += amount;
        player->shield = min(player->shield, 1.0f);
        player->shield = max(oldShield, player->shield);

        float actualRegen = player->shield - oldShield;

        if (actualRegen > 0.1f) {
            // sm64_play_sound_global(SOUND_GENERAL_HEART_SPIN);
        }
    }
    
    void updateShieldRegen(Engine::Entity* player) {
        if (!player) return;
        
        uint32_t action = marioState.action;
        bool risingEdge = (action != prevAction);
        prevAction = action;

        if (!risingEdge) return;

        float regen = 0.0f;
        switch (action) {
            case ACT_SIDE_FLIP:    regen = SIDE_FLIP_REGEN;   break;
            case ACT_WALL_KICK_AIR: regen = WALL_KICK_REGEN;  break;
            case ACT_TRIPLE_JUMP:  regen = TRIPLE_JUMP_REGEN; break;
            default: return;
        }

        regenerateShield(player, regen, true);
    }

}
