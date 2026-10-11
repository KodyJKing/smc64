#include "MarioToCheif.hpp"

#include "../MarioState.hpp"
#include "engine/halo1.hpp"
#include "math/Vectors.hpp"
#include "../Coordinates.hpp"
#include "../collision/MarioBSPChunk.hpp"

#include "libsm64.h"

namespace Mod::Mario {

    void marioToCheif() {
        auto player = Engine::getPlayerEntity();
        if (player && player->worldBones.count() > 0) {
            auto bones = player->worldBones.get(player, 0);
            auto pos = bones[0].pos;
            
            Vec3 marioWorldPos = Coordinates::haloToMario(pos);
            Vec3i targetChunk  = Coordinates::marioChunkForPosition(marioWorldPos);
            if (targetChunk.x != marioChunk.x ||
                targetChunk.y != marioChunk.y ||
                targetChunk.z != marioChunk.z) {
                MarioBSPChunk::reloadFor(targetChunk);
            }
            Vec3 local = Coordinates::marioWorldToLocal(marioWorldPos, marioChunk);
            sm64_set_mario_position(marioId, local.x, local.y, local.z);
            sm64_mario_heal(marioId, 0xFF);
        }
    }

}
