#include "haloce/ui/DrawEntityCollision.hpp"
#include "overlay/ESP.hpp"
#include "DrawBSP.hpp"

namespace HaloCE::Mod::UI  {
    void drawEntityCollision(Halo1::Entity* entity) {
        namespace ESP = Overlay::ESP;
        Camera &camera = ESP::camera;

        Vec3 x = entity->fwd.normalize();
        Vec3 z = entity->up.normalize();
        Vec3 y = z.cross(x).normalize();

        auto collisionTag = getCollisionGeometryTag(entity->tag());
        if (collisionTag == nullptr) return;
        auto collisionData = (Halo1::ObjectCollisionTagData*) collisionTag->getData();
        if (collisionData == nullptr) return;
        
        auto nodeCount = collisionData->collisionNodes.count;
        for (uint16_t i = 0; i < nodeCount; i++) {
            auto node = collisionData->collisionNodes.get<Halo1::CollisionNode>(i);
            if (node == nullptr) continue;

            auto bsp = node->collisionBsps.get<Halo1::CollisionBSP>(0);
            if (bsp == nullptr) continue;

            // Blow up the BSPs in a sphere around the entity for visibility until we can associate them with a bone transform.
            float phi = i * (3.14159f * 2.0f / nodeCount);
            float theta = i * (50.0f / nodeCount);
            float radius = 1.0f;
            float cx = radius * std::cosf(phi) * std::sinf(theta);
            float cy = radius * std::sinf(phi) * std::sinf(theta);
            float cz = radius * std::cosf(theta);

            Vec3 origin = entity->pos + Vec3{cx, cy, cz};

            
            HaloCE::Mod::UI::drawBSP(bsp, origin, x, y, z);
            
            Vec3 toOrigin = (origin - camera.pos).normalize();
            float alignment = toOrigin.dot(camera.fwd);
            float alpha = (alignment - 0.999f) * 1000.0f * 255.0f;
            if (alpha < 10) continue;
            uint8_t ialpha = alpha;

            std::string name = node->name;
            std::string text = name + "\n"
                 + "Index: " + std::to_string(i) + "\n"
                 + "BSP count: " +  std::to_string(node->collisionBsps.count);
            Overlay::ESP::drawText(origin, text.c_str(), IM_COL32(255, 255, 255, ialpha));
        }
    }
}
