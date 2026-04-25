#include "haloce/ui/DrawEntity.hpp"

#include "DrawBones.hpp"
#include "DrawEntityCollision.hpp"

namespace HaloCE::Mod::UI  {
    void drawEntity(Halo1::Entity* entity) {
        // Draw bones
        drawBones(entity);
        // Draw collision
        // drawEntityCollision(entity);
    }
}
