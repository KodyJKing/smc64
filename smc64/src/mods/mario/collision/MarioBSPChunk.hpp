#pragma once

#include "math/Vectors.hpp"

namespace Mod::Mario::MarioBSPChunk {

    // Hysteresis: only reload static geometry once Mario passes the loaded chunk's
    // boundary by this many mario units. Tunable for testing.
    extern float reloadMargin;

    // The chunk that the currently uploaded static surfaces are expressed relative to.
    Vec3i getLoadedChunk();

    // Build and upload static surfaces for the neighborhood around `initialChunk`,
    // then record it as the loaded chunk. Call once after libsm64 is initialized.
    void init(Vec3i initialChunk);

    // Free old static buffer, rebuild surfaces for the neighborhood around `newChunk`,
    // upload via sm64_static_surfaces_load, and notify DynamicGeometry.
    void reloadFor(Vec3i newChunk);

    // Called once per tick (after sm64_mario_tick). Recenters Mario's local coords
    // when he crosses his current chunk, and triggers a hysteresis reload when
    // marioChunk drifts from the loaded chunk.
    void maintain();

    // Release the static surface buffer (call on shutdown).
    void free();

    // Draw the currently loaded static surfaces as wireframe triangles (debug only).
    void debugRender();

    // Used to determine if it is safe to tick Mario.
    bool hasValidLoadedChunk();
}
