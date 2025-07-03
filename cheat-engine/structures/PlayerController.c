#include <stdint.h>

#pragma pack(push, 1)
struct PlayerController {
    uint8_t pad_0000[16];     // 0000
    uint32_t playerHandle;    // 0010
    uint32_t actionsBitfield; // 0014
    uint8_t pad_0018[4];      // 0018
    float yaw;                // 001C
    float pitch;              // 0020
    float walkY;              // 0024
    float walkX;              // 0028
    float gunTrigger;         // 002C
    uint8_t pad_0030[8];      // 0030
    float targetIndicator;    // 0038
    uint8_t pad_003C[360];    // 003C
    uint32_t targetHandle;    // 01A4
};
#pragma pack(pop)

