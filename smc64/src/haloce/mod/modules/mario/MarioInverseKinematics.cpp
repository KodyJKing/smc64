#include "MarioInverseKinematics.hpp"

namespace HaloCE::Mod::Mario::InverseKinematics {

    void inferBoneLengths(IKRequest& request) {
        for (size_t i = 0; i < request.bones.size() - 1; i++) {
            Vec3 start = request.bones[i].initialTransform.pos;
            Vec3 end = request.bones[i + 1].initialTransform.pos;
            request.bones[i].length = (end - start).length();
        }
    }

    void setupCurrentTransformsFromInitial(IKRequest& request) {
        for (auto& bone : request.bones) {
            bone.currentTransform = bone.initialTransform;
        }
    }

    void addPivotOffset(IKRequest& request, float sign) {
        for (auto& bone : request.bones) {
            Vec3 worldPivotOffset = bone.currentTransform.x * bone.pivotOffset.x + bone.currentTransform.y * bone.pivotOffset.y + bone.currentTransform.z * bone.pivotOffset.z;
            bone.currentTransform.pos += worldPivotOffset * sign;
            bone.initialTransform.pos += worldPivotOffset * sign;
        }
    }

    void solveLimbIK(IKRequest& request) {
        
        addPivotOffset(request, 1.0f);
        inferBoneLengths(request);
        setupCurrentTransformsFromInitial(request);

        // Solve via FABRIK. No joint constraints for now, just length constraints.
        const int iterationLimit = 10;
        for (int i = 0; i < iterationLimit; i++) {
            
            // Backwards pass: set end effector to target and move down the chain.
            // Special handling for last bone.
            Vec3 dir = (request.targetPosition - request.bones.back().currentTransform.pos).normalize();
            request.bones.back().currentTransform.pos = request.targetPosition - dir * request.bones.back().length;
            for (int j = (int)request.bones.size() - 2; j >= 0; j--) {
                Vec3 dir = (request.bones[j].currentTransform.pos - request.bones[j + 1].currentTransform.pos).normalize();
                request.bones[j].currentTransform.pos = request.bones[j + 1].currentTransform.pos + dir * request.bones[j].length;
            }

            // Forwards pass: set root to initial position and move up the chain.
            request.bones[0].currentTransform.pos = request.bones[0].initialTransform.pos;
            for (size_t j = 1; j < request.bones.size(); j++) {
                Vec3 dir = (request.bones[j].currentTransform.pos - request.bones[j - 1].currentTransform.pos).normalize();
                request.bones[j].currentTransform.pos = request.bones[j - 1].currentTransform.pos + dir * request.bones[j - 1].length;
            }
        }

        // Project old bone orientations onto new positions.
        for (int i = 0; i < (int)request.bones.size() - 1; i++) {
            Vec3 newXDir = (request.bones[i + 1].currentTransform.pos - request.bones[i].currentTransform.pos).normalize();
            request.bones[i].currentTransform.x = newXDir;

            // Re-orthonormalize y and z based on original orientation:
            Vec3 yDir = request.bones[i].initialTransform.y;
            Vec3 newZDir = newXDir.cross(yDir).normalize();
            Vec3 newYDir = newZDir.cross(newXDir).normalize();
            request.bones[i].currentTransform.y = newYDir;
            request.bones[i].currentTransform.z = newZDir;
        }
        
        // Special handling for last bone.
        {
            Vec3 newXDir = (request.targetPosition - request.bones.back().currentTransform.pos).normalize();

            request.bones.back().currentTransform.x = newXDir;

            // Re-orthonormalize y and z based on original orientation:
            Vec3 yDir = request.bones.back().initialTransform.y;
            Vec3 newZDir = newXDir.cross(yDir).normalize();
            Vec3 newYDir = newZDir.cross(newXDir).normalize();
            request.bones.back().currentTransform.y = newYDir;
            request.bones.back().currentTransform.z = newZDir;
        }

        addPivotOffset(request, -1.0f);
    }

    void applyMarioIK(MarioIKRequest& request) {
        std::vector<const char*> boneNames;
        std::vector<Vec3> pivotOffsets;
        if (request.limb == MarioIKRequest::Limb::LeftArm) {
            boneNames = { "left_arm", "left_forearm", "left_hand" };
            pivotOffsets = {
                Vec3{ 0.01f, 0.01f, 0.01f },
                Vec3{ 0.01f, 0.01f, 0.01f },
                Vec3{ 0.01f, 0.0f, 0.00f },
            };
        } else {
            boneNames = { "right_arm", "right_forearm", "right_hand" };
            pivotOffsets = {
                Vec3{ 0.0f, -0.01f, 0.01f },
                Vec3{ 0.0f, -0.01f, 0.01f },
                Vec3{ 0.0f, 0.0f, 0.0f },
            };
        }

        IKRequest ikRequest;
        for (int i = 0; i < (int)boneNames.size(); i++) {
            Halo1::WorldTransform initialTransform = getMarioBoneByName(boneNames[i]);
            float length = i == 2 ? 0.05f : -1.0f;
            Vec3 pivotOffset = pivotOffsets[i];
            ikRequest.bones.push_back({ initialTransform, length, pivotOffset });
        }

        ikRequest.targetPosition = request.targetPosition;
        solveLimbIK(ikRequest);

        // Apply solved IK transforms back to Mario's skeleton.
        for (size_t i = 0; i < ikRequest.bones.size(); i++) {
            const auto& ikBone = ikRequest.bones[i];
            const char* boneName = boneNames[i];
            Halo1::WorldTransform* marioBone = getMarioBonePointerByName(boneName);
            if (marioBone) {
                *marioBone = ikBone.currentTransform;
            }
        }
    }

}
