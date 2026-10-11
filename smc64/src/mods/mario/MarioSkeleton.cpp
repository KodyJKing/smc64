#include "MarioSkeleton.hpp"

#include <vector>

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "spark/overlay/ESP.hpp"
#include "Coordinates.hpp"
#include "MarioState.hpp"

#include <unordered_map>
#include <string>

#define DEBUG_MARIO_SKELETON 1

#if DEBUG_MARIO_SKELETON
#include <iostream>
#define LOG(msg) std::cout << msg << std::endl;
#else
#define LOG(msg)
#endif

namespace Mod::Mario {
    struct MarioBone {
        char name[64];
    };

    std::vector<MarioBone> marioBones = {
        {"frame base"},
        {"frame pelvis"},
        {"frame chest"},
        {"frame head"},
        {"frame left_shoulder"},
        {"frame left_arm"},
        {"frame left_forearm"},
        {"frame left_hand"},
        {"frame right_shoulder"},
        {"frame right_arm"},
        {"frame right_forearm"},
        {"frame right_hand"},
        {"frame left_hip"},
        {"frame left_thigh"},
        {"frame left_calf"},
        {"frame left_foot"},
        {"frame right_hip"},
        {"frame right_thigh"},
        {"frame right_calf"},
        {"frame right_foot"},

        // Additional bones for equipment like rideable shield.
        {"frame shield"},
        {"frame powder_keg"}
    };

    std::vector<std::string> haloBones = {
        "frame base",
        "frame chest",
        "frame head",
        "frame right_arm",
        "frame right_calf",
        "frame right_foot",
        "frame right_forearm",
        "frame right_hand",
        "frame right_hip",
        "frame right_shoulder",
        "frame right_thigh",
        "frame pelvis",
        "frame powder_keg",
        "frame left_arm",
        "frame left_calf",
        "frame left_foot",
        "frame left_forearm",
        "frame left_hand",
        "frame left_hip",
        "frame left_shoulder",
        "frame left_thigh",
        "frame shield",
    };

    bool haloBonesInvInitialized = false;
    std::unordered_map<std::string, size_t> haloBonesInv;
    void initializeHaloBonesInv() {
        if (!haloBonesInvInitialized) {
            for (size_t i = 0; i < haloBones.size(); i++) {
                haloBonesInv[haloBones[i]] = i;
            }
            haloBonesInvInitialized = true;
        }
    }

    size_t getHaloBoneIndexByName(const std::string& name) {
        initializeHaloBonesInv();
        auto it = haloBonesInv.find(name);
        if (it != haloBonesInv.end()) {
            return it->second;
        }
        return static_cast<size_t>(-1);
    }

    std::vector<Engine::Transform> marioPose = {
        {0}, // base
        {0}, // pelvis
        {0}, // chest
        {0}, // head

        {0}, // left_shoulder
        {0}, // left_arm
        {0}, // left_forearm
        {0}, // left_hand

        {0}, // right_shoulder
        {0}, // right_arm
        {0}, // right_forearm
        {0}, // right_hand

        {0}, // left_hip
        {0}, // left_thigh
        {0}, // left_calf
        {0}, // left_foot

        {0}, // right_hip
        {0}, // right_thigh
        {0}, // right_calf
        {0}, // right_foot

        // Additional bones for equipment like rideable shield.
        Engine::Transform::identity(),
        Engine::Transform::identity()
    };

    void updateMarioPose(SM64Matrix4f* marioBoneMatrices) {
        for (size_t i = 0; i < SM64_MARIO_BONE_COUNT; i++) {
            auto& marioMat = marioBoneMatrices[i];

            size_t j = getHaloBoneIndexByName(marioBones[i].name);
            auto& transform = marioPose[j];

            Vec3 marioX = Vec3{marioMat.m[0][0], marioMat.m[0][1], marioMat.m[0][2]};
            Vec3 marioY = Vec3{marioMat.m[1][0], marioMat.m[1][1], marioMat.m[1][2]};
            Vec3 marioZ = Vec3{marioMat.m[2][0], marioMat.m[2][1], marioMat.m[2][2]};
            Vec3 marioPos = Vec3{marioMat.m[3][0], marioMat.m[3][1], marioMat.m[3][2]};

            Vec3 haloX = Coordinates::marioDirectionToHalo(marioX);
            Vec3 haloY = Coordinates::marioDirectionToHalo(marioY);
            Vec3 haloZ = Coordinates::marioDirectionToHalo(marioZ);
            Vec3 haloPos = Coordinates::marioLocalToHaloWorld(marioPos, marioChunk);

            transform.pos = haloPos;
            transform.x = haloX.normalize();
            transform.y = haloY.normalize();
            transform.z = haloZ.normalize() * -1.0f;
            transform.w = 0.8f;
        }
    }

    Engine::Transform* getMarioBonePointerByName(const char* name) {
        for (size_t i = 0; i < marioBones.size(); i++) {
            if (strcmp(haloBones[i].c_str(), name) == 0) {
                return &marioPose[i];
            }
        }
        return nullptr;
    }

    Engine::Transform getMarioBoneByName(const char* name) {
        return *getMarioBonePointerByName(name);
    }

}
