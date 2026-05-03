#pragma once

#include <stdint.h>
#include <optional>
#include "camera.hpp"
#include "camera_controller.hpp"
#include "entity/index.hpp"

namespace Halo1 {
    namespace PlayerActionFlags {
        const uint32_t crouch     = 1 << 0;
        const uint32_t jump       = 1 << 1;
        const uint32_t flash      = 1 << 4;
        const uint32_t melee      = 1 << 7;
        const uint32_t reload     = 1 << 10;
        const uint32_t shoot      = 1 << 11;
        const uint32_t grenade1   = 1 << 12;
        const uint32_t grenade2   = 1 << 13;
    }
    struct PlayerController {
        uint8_t pad_0000[16];  // 0000
        uint32_t playerHandle; // 0010
        uint32_t actions;      // 0014
        uint8_t pad_0018[4];   // 0018
        float yaw;             // 001C
        float pitch;           // 0020
        float walkY;           // 0024
        float walkX;           // 0028
        float gunTrigger;      // 002C
        uint8_t pad_0030[8];   // 0030
        float targetIndicator; // 0038
        uint8_t pad_003C[360]; // 003C
        uint32_t targetHandle; // 01A4
    };

    Camera* getPlayerCameraPointer();
    CameraController *getPlayerCameraControllerPointer();
    void enterThirdPerson();
    uint32_t getPlayerHandle();
    PlayerController * getPlayerControllerPointer();
    
    EntityRecord* getPlayerRecord();
    Entity *getPlayerEntity();
    std::optional<Vec3> getPlayerPosition();
    bool isPlayerHandle( uint32_t entityHandle );
    bool isPlayerControlled( EntityRecord* rec );
}
