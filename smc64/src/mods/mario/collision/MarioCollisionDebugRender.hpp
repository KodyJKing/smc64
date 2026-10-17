#pragma once

#define DEBUG_MARIO_COLLISION 1
// #define DEBUG_MARIO_COLLISION_HISTORY 1
#define COLLISION_HISTORY_SIZE 16

namespace Mod::Mario {
namespace CollisionDebugRender {
    #ifdef DEBUG_MARIO_COLLISION
    // Query sm64 surfaces for the current Mario position, update history, and draw ESP trails.
    void render();
    // Render the collision section inside the Mario debug ImGui window.
    void renderImGuiSection();
    #endif // DEBUG_MARIO_COLLISION
}
}
