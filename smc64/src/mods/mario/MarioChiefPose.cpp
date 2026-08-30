#include "math/Vectors.hpp"
#include "spark/mod/ModId.hpp"
#include "MarioSkeleton.hpp"
#include "engine/halo1.hpp"
#include "spark/hook/Hooks.hpp"

#include <vector>
#include <string>

// #define BONE_MAPPING_EDITOR 1

#ifdef BONE_MAPPING_EDITOR
#include "imgui.h"
#include "spark/Spark.hpp"
#endif

namespace Mod::Mario::MarioChiefPose {

    struct BoneMapping {
        std::string marioBone;
        uint16_t chiefBone;
        Engine::Transform relativeTransform;
#ifdef BONE_MAPPING_EDITOR
        Vec3 eulerDeg;
        Vec3 scale;

        BoneMapping(std::string marioBone, uint16_t chiefBone, Engine::Transform relativeTransform)
            : marioBone(std::move(marioBone)), chiefBone(chiefBone), relativeTransform(relativeTransform)
        {
            scale.x = relativeTransform.x.length();
            scale.y = relativeTransform.y.length();
            scale.z = relativeTransform.z.length();
            Vec3 nx = (scale.x > 1e-6f) ? relativeTransform.x / scale.x : Vec3{1.f, 0.f, 0.f};
            Vec3 nz = (scale.z > 1e-6f) ? relativeTransform.z / scale.z : Vec3{0.f, 0.f, 1.f};
            eulerDeg = Vec3::orientationToEuler_YXZ(nx, nz);
        }
#endif
    };

    // Mapping from Mario bones to Cheif bones.
    std::vector<BoneMapping> marioToChiefBoneMap = {
        {"frame pelvis", 0, Engine::Transform::identity()},
        {"frame head", 12, Engine::Transform::identity()},
        {"frame chest", 6, Engine::Transform::identity()},

        {"frame right_thigh", 1, Engine::Transform::identity()},
        {"frame right_calf", 4, Engine::Transform::identity()},
        {"frame right_foot", 8, Engine::Transform{1.000000f, {0.290702f,0.956814f,0.000000f}, {0.956814f,-0.290702f,0.000000f}, {0.000000f,-0.000000f,-1.000000f}, {-0.005000f,0.010000f,0.031000f}}},

        {"frame left_thigh", 2, Engine::Transform::identity()},
        {"frame left_calf", 5, Engine::Transform::identity()},
        {"frame left_foot", 11, Engine::Transform{1.000000f, {0.290702f,0.956814f,-0.000000f}, {0.956814f,-0.290702f,0.000000f}, {-0.000000f,-0.000000f,-1.000000f}, {-0.005000f,0.026000f,-0.021000f}}},

        {"frame right_arm", 13, Engine::Transform::identity()},
        {"frame right_forearm", 15, Engine::Transform::identity()},
        {"frame right_hand", 17, Engine::Transform::identity()},

        {"frame left_arm", 14, Engine::Transform::identity()},
        {"frame left_forearm", 16, Engine::Transform::identity()},
        {"frame left_hand", 18, Engine::Transform::identity()}
    };
    
    // Pose Mario using Chief's pose.
    void updatePose() {
        auto playerEntity = Engine::getPlayerEntity();
        if (!playerEntity || !Engine::entityValid(playerEntity)) return;

        auto bones = playerEntity->worldBones.get(playerEntity, 0);
        if (!bones) return;
        auto boneCount = playerEntity->worldBones.count();
        if (boneCount == 0) return;

        // Displacement accounts for 1-frame lag when copying Chief's pose to Mario.
        const float extrapolationFactor = 1.0f;
        Vec3 playerVel = Engine::getPlayerVelocity().value_or(Vec3{0.f, 0.f, 0.f});
        Vec3 displacement = playerVel * extrapolationFactor;
        
        for (auto& mapping : marioToChiefBoneMap) {
            auto marioBone = getMarioBonePointerByName(mapping.marioBone.c_str());
            if (!marioBone) continue;

            if (mapping.chiefBone >= boneCount) continue;
            auto chiefBone = bones[mapping.chiefBone];

            // Apply the relative transform from Mario bone to Cheif bone.
            marioBone->pos = chiefBone.transformPoint(mapping.relativeTransform.pos) + displacement;
            marioBone->x = chiefBone.transformVec(mapping.relativeTransform.x);
            marioBone->y = chiefBone.transformVec(mapping.relativeTransform.y);
            marioBone->z = chiefBone.transformVec(mapping.relativeTransform.z);
        }
    }

#ifdef BONE_MAPPING_EDITOR
    static bool s_inputEnabled = false;
#endif

    void initHandlers(Spark::ModId modId) {
#ifdef BONE_MAPPING_EDITOR
        Spark::UpdatePlayerControlsAndLook::addHandler(modId, +[](void*, Spark::UpdatePlayerControlsAndLook::Cursor next, float p1, uint32_t p2) {
            if (!s_inputEnabled)
                next(p1, p2);
        }, nullptr, 10);
#else
        (void)modId;
#endif
    }

    void render() {
        #ifdef BONE_MAPPING_EDITOR
        if (!Spark::showDebugOverlay) return;

        if (ImGui::IsKeyPressed(ImGuiKey_Tab, false))
            s_inputEnabled = !s_inputEnabled;

        static int s_selectedIdx = 0;

        ImGui::Begin("Bone Mapping Editor", nullptr, ImGuiWindowFlags_AlwaysAutoResize);

        if (s_inputEnabled) {
            ImVec2 mp = ImGui::GetMousePos();
            ImDrawList* dl = ImGui::GetForegroundDrawList();
            const float R = 8.0f;
            const float G = 2.0f;
            const ImU32 kCursorColor  = IM_COL32(255, 255, 255, 220);
            const ImU32 kCursorShadow = IM_COL32(0,   0,   0,   120);
            dl->AddLine({mp.x - R + 1, mp.y + 1}, {mp.x + R + 1, mp.y + 1}, kCursorShadow, G);
            dl->AddLine({mp.x + 1, mp.y - R + 1}, {mp.x + 1, mp.y + R + 1}, kCursorShadow, G);
            dl->AddLine({mp.x - R, mp.y}, {mp.x + R, mp.y}, kCursorColor, G);
            dl->AddLine({mp.x, mp.y - R}, {mp.x, mp.y + R}, kCursorColor, G);
        }

        ImGui::TextColored(
            s_inputEnabled ? ImVec4(0.2f, 1.f, 0.2f, 1.f) : ImVec4(1.f, 0.6f, 0.1f, 1.f),
            s_inputEnabled ? "[Tab] Input: captured" : "[Tab] Input: pass-through");

        // Bone list
        if (ImGui::BeginListBox("##bones", ImVec2(200.f, ImGui::GetTextLineHeightWithSpacing() * (float)marioToChiefBoneMap.size() + 4.f))) {
            for (int i = 0; i < (int)marioToChiefBoneMap.size(); i++) {
                bool selected = (i == s_selectedIdx);
                if (ImGui::Selectable(marioToChiefBoneMap[i].marioBone.c_str(), selected))
                    s_selectedIdx = i;
                if (selected)
                    ImGui::SetItemDefaultFocus();
            }
            ImGui::EndListBox();
        }

        if (s_selectedIdx >= 0 && s_selectedIdx < (int)marioToChiefBoneMap.size()) {
            auto& bm = marioToChiefBoneMap[s_selectedIdx];

            ImGui::Separator();
            ImGui::Text("Mario: %s  ->  Chief[%d]", bm.marioBone.c_str(), (int)bm.chiefBone);
            ImGui::Separator();

            bool changed = false;

            ImGui::SeparatorText("Rotation (deg)");
            changed |= ImGui::SliderFloat("Yaw",   &bm.eulerDeg.y, -180.f, 180.f, "%.1f");
            changed |= ImGui::SliderFloat("Pitch", &bm.eulerDeg.x, -180.f, 180.f, "%.1f");
            changed |= ImGui::SliderFloat("Roll",  &bm.eulerDeg.z, -180.f, 180.f, "%.1f");

            ImGui::SeparatorText("Scale");
            changed |= ImGui::SliderFloat("Scale X", &bm.scale.x, 0.01f, 5.f, "%.3f");
            changed |= ImGui::SliderFloat("Scale Y", &bm.scale.y, 0.01f, 5.f, "%.3f");
            changed |= ImGui::SliderFloat("Scale Z", &bm.scale.z, 0.01f, 5.f, "%.3f");

            ImGui::SeparatorText("Translation");
            changed |= ImGui::SliderFloat("Trans X", &bm.relativeTransform.pos.x, -.5f, .5f, "%.3f");
            changed |= ImGui::SliderFloat("Trans Y", &bm.relativeTransform.pos.y, -.5f, .5f, "%.3f");
            changed |= ImGui::SliderFloat("Trans Z", &bm.relativeTransform.pos.z, -.5f, .5f, "%.3f");

            if (changed) {
                Matrix3 rot = Matrix3::fromEulerYXZ(bm.eulerDeg);
                bm.relativeTransform.x = rot.columns.x * bm.scale.x;
                bm.relativeTransform.y = rot.columns.y * bm.scale.y;
                bm.relativeTransform.z = rot.columns.z * bm.scale.z;
                bm.relativeTransform.w = 1.0f;
            }

            if (ImGui::Button("Reset")) {
                bm.eulerDeg            = {0.f, 0.f, 0.f};
                bm.scale               = {1.f, 1.f, 1.f};
                bm.relativeTransform   = Engine::Transform::identity();
            }
            ImGui::SameLine();
            if (ImGui::Button("Copy transform")) {
                const auto& t = bm.relativeTransform;
                char buf[512];
                snprintf(buf, sizeof(buf),
                    "{\"%s\", %d, Engine::Transform{%.6ff, "
                    "{%.6ff,%.6ff,%.6ff}, "
                    "{%.6ff,%.6ff,%.6ff}, "
                    "{%.6ff,%.6ff,%.6ff}, "
                    "{%.6ff,%.6ff,%.6ff}}},",
                    bm.marioBone.c_str(), (int)bm.chiefBone,
                    t.w,
                    t.x.x, t.x.y, t.x.z,
                    t.y.x, t.y.y, t.y.z,
                    t.z.x, t.z.y, t.z.z,
                    t.pos.x, t.pos.y, t.pos.z);
                ImGui::SetClipboardText(buf);
            }
        }

        ImGui::End();
        #endif
    }

}
