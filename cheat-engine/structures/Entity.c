#include <stdint.h>

#pragma pack(push, 1)
struct Entity {
    uint16_t tagID;                     // 0000
    uint8_t pad_0002[18];               // 0002
    int32_t ageMilis;                   // 0014
    float posX;                         // 0018
    float posY;                         // 001C
    float posZ;                         // 0020
    float velX;                         // 0024
    float velY;                         // 0028
    float velZ;                         // 002C
    float fwdX;                         // 0030
    float fwdY;                         // 0034
    float fwdZ;                         // 0038
    float upX;                          // 003C
    float upY;                          // 0040
    float upZ;                          // 0044
    float angularVelX;                  // 0048
    float angularVelY;                  // 004C
    float angularVelZ;                  // 0050
    uint8_t pad_0054[8];                // 0054
    float rootBoneX;                    // 005C
    float rootBoneY;                    // 0060
    float rootBoneZ;                    // 0064
    uint8_t pad_0068[8];                // 0068
    int16_t entityCategory;             // 0070
    uint8_t pad_0072[14];               // 0072
    uint32_t controllerHandle;          // 0080
    uint8_t pad_0084[8];                // 0084
    uint16_t animID;                    // 008C
    int16_t animFrame;                  // 008E
    uint8_t pad_0090[12];               // 0090
    float health;                       // 009C
    float shield;                       // 00A0
    uint8_t pad_00A4[44];               // 00A4
    uint32_t vehicleHandle;             // 00D0
    uint32_t childHandle;               // 00D4
    uint32_t parentHandle;              // 00D8
    uint8_t pad_00DC[240];              // 00DC
    uint32_t previousWeaponOwnerHandle; // 01CC
    uint8_t pad_01D0[48];               // 01D0
    uint32_t projectileParentHandle;    // 0200
    float heat;                         // 0204
    float plasmaUsed;                   // 0208
    float fuse;                         // 020C
    float lookX;                        // 0210
    float lookY;                        // 0214
    float lookZ;                        // 0218
    float projectileAge;                // 021C
    uint8_t pad_0220[8];                // 0220
    int8_t ticksSinceLastFired;         // 0228
    uint8_t pad_0229[23];               // 0229
    float plasmaCharge;                 // 0240
    uint8_t pad_0244[61];               // 0244
    uint8_t weaponIndex;                // 0281
    uint8_t pad_0282[1];                // 0282
    uint8_t grenadeAnim;                // 0283
    uint8_t weaponAnim;                 // 0284
    uint8_t pad_0285[1];                // 0285
    int16_t ammo;                       // 0286
    uint8_t pad_0288[2];                // 0288
    int16_t clipAmmo;                   // 028A
    uint8_t pad_028C[112];              // 028C
    int8_t frags;                       // 02FC
    int8_t plasmas;                     // 02FD
    uint8_t pad_02FE[6];                // 02FE
    uint32_t vehicleRiderHandle;        // 0304
    uint32_t vehicleRiderHandle2;       // 0308
};
#pragma pack(pop)

