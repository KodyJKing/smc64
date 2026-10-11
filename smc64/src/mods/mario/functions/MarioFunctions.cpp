#include "MarioFunctions.hpp"
#include "../MarioState.hpp"
#include "../Coordinates.hpp"
#include "decomp/sm64.h"

namespace Mod::Mario {

    void launchMario(const Vec3& velocity, bool setFaceAngle) {
        Vec3 marioVelocity = Coordinates::haloToMario(velocity);

        float yaw = atan2f(velocity.x, velocity.y);
        float pitch = atan2f(-velocity.z, sqrtf(velocity.x * velocity.x + velocity.y * velocity.y));

        if (setFaceAngle) {
            sm64_set_mario_angle(marioId, 0, yaw, pitch);
            sm64_set_mario_velocity(marioId, marioVelocity.x, marioVelocity.y, marioVelocity.z);
        } else {
            sm64_set_mario_angle(marioId, 0, marioState.faceAngle, pitch);
            sm64_set_mario_velocity(marioId, 0, marioVelocity.y, 0);
        }

        sm64_set_mario_forward_velocity(marioId, marioVelocity.length());
    }

}
