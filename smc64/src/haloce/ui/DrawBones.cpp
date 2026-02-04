#include "haloce/ui/DrawBones.hpp"
#include "overlay/ESP.hpp"

namespace HaloCE::Mod::UI  {
    void drawBones(Halo1::Entity* entity) {
        namespace ESP = Overlay::ESP;
        Camera &camera = ESP::camera;

        auto boneCount = entity->worldBones.count();
        Halo1::WorldTransform* worldBones = entity->worldBones.get( entity, 0 );

        for ( uint16_t i = 0; i < boneCount; i++ ) {
            auto& bone = worldBones[i];

            Vec3 pos = bone.pos;

            float r = 0.0125f;

            ESP::drawLine(pos, pos + bone.x * r, IM_COL32(255, 0, 0, 255)); // Forward - Red
            ESP::drawLine(pos, pos + bone.y * r, IM_COL32(0, 255, 0, 255)); // Left - Green
            ESP::drawLine(pos, pos + bone.z * r, IM_COL32(0, 0, 255, 255)); // Up - Blue
        }
    }
}
