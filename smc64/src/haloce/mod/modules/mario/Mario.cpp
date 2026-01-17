#include "Mario.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "overlay/ESP.hpp"

#include "libsm64.h"

#include <cstdlib>
#include <cstdio>
#include <cstdint>

#include <Windows.h>

#include <filesystem>
#include "../../../halo1/Halo1.hpp"
#include "Mario.hpp"
#include "MarioInput.hpp"
#include "StaticGeometry.hpp"
#include "math/Vectors.hpp"
#include "Coordinates.hpp"

#include "../FreeCam.hpp"

namespace HaloCE::Mod::Mario {

    // Internal:

    uint8_t *texture = nullptr;
    uint8_t *rom;
    size_t romSize = 0;

    SM64Surface* staticSurfaces = nullptr;
    size_t staticSurfacesCount = 0;

    int32_t marioId = -1;
    struct SM64MarioInputs marioInputs;
    struct SM64MarioState marioState;
    struct SM64MarioGeometryBuffers marioGeometry;

    uint8_t* readRomFile(const char *path, size_t *fileLength) {
        FILE *f = fopen(path, "rb");
        if (!f) {
            printf("\nFailed to read ROM file \"%s\"\n\n", path);
            return nullptr;
        }

        fseek(f, 0, SEEK_END);
        size_t length = (size_t)ftell(f);
        rewind(f);
        uint8_t *buffer = (uint8_t*)malloc(length + 1);
        fread(buffer, 1, length, f);
        buffer[length] = 0;
        fclose(f);

        if (fileLength) *fileLength = length;

        return buffer;
    }

    void debugPrint(const char *msg) {
        printf("%s\n", msg);
    }

    void initMario() {
        // Create a Mario instance at the origin.
        if (marioId < 0) {
            marioId = sm64_mario_create(99999.0f, 99999.0f, 99999.0f);
            auto playerPos = Halo1::getPlayerPosition();
            if (playerPos.has_value()) {
                auto pos = playerPos.value();
                // Convert Halo CE coordinates to Super Mario 64 coordinates
                auto marioPos = Coordinates::haloToMario(pos);
                sm64_set_mario_position(marioId, marioPos.x, marioPos.y, marioPos.z);
            }
        }
        if (marioId < 0) {
            printf("Failed to create Mario instance.\n");
            return;
        }
        printf("Created Mario instance with ID: %d\n", marioId);

        // Initialize Mario state and geometry buffers.
        marioInputs = {};
        marioState = {};
        marioGeometry.position = (float*)malloc(sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES);
        marioGeometry.color = (float*)malloc(sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES);
        marioGeometry.normal = (float*)malloc(sizeof(float) * 9 * SM64_GEO_MAX_TRIANGLES);
        marioGeometry.uv = (float*)malloc(sizeof(float) * 6 * SM64_GEO_MAX_TRIANGLES);
        marioGeometry.numTrianglesUsed = 0;

        // Print buffer locations
        printf("Mario geometry buffers initialized:\n");
        printf(" - Position buffer: %p\n", (void*)marioGeometry.position);
        printf(" - Color buffer: %p\n", (void*)marioGeometry.color);
        printf(" - Normal buffer: %p\n", (void*)marioGeometry.normal);
        printf(" - UV buffer: %p\n", (void*)marioGeometry.uv);
    }

    void initTestLevel() {
        // // Just create a square that spans -1,1 in the x and z axes, with a height of 0.
        // staticSurfacesCount = 2;
        // staticSurfaces = (SM64Surface*)malloc(sizeof(SM64Surface) * staticSurfacesCount);
        // staticSurfaces[0] = { 0, 0, 0, { -1, 0, -1, 1, 0, -1, 1, 0, 1 } };
        // staticSurfaces[1] = { 0, 0, 0, { -1, 0, 1, 1, 0, 1, 1, 0, -1 } };
        // sm64_static_surfaces_load(staticSurfaces, staticSurfacesCount);
        // printf("Created test level with %zu static surfaces.\n", staticSurfacesCount);

        // Load Halo CE static geometry and convert it to Super Mario 64 format
        auto surfaceVector = HaloCE::Mod::StaticGeometry::haloGeometryToMario();
        staticSurfacesCount = surfaceVector.size();
        if (staticSurfacesCount == 0) {
            printf("No static surfaces found in Halo CE geometry.\n");
            return;
        }

        staticSurfaces = (SM64Surface*)malloc(sizeof(SM64Surface) * staticSurfacesCount);
        // memccpy(staticSurfaces, surfaceVector.data(), sizeof(SM64Surface), staticSurfacesCount);
        memcpy(staticSurfaces, surfaceVector.data(), sizeof(SM64Surface) * staticSurfacesCount);

        sm64_static_surfaces_load(staticSurfaces, staticSurfacesCount);
        printf("Loaded %zu static surfaces from Halo CE geometry.\n", staticSurfacesCount);
    }

    // Public:
    
    void init() {
        #define ENABLE_MARIO 1
        #ifdef ENABLE_MARIO
        // Get location of host exe file using Windows API
        char path[MAX_PATH];
        GetModuleFileNameA(nullptr, path, MAX_PATH);
        std::filesystem::path modulePath = path;
        std::filesystem::path romPath = modulePath.parent_path() / "baserom.us.z64";
        std::string romPathStr = romPath.string();
        printf("Module path: %s\n", romPathStr.c_str());

        rom = readRomFile(romPathStr.c_str(), &romSize);
        if (rom == nullptr) return;
        texture = (uint8_t*)malloc( 4 * SM64_TEXTURE_WIDTH * SM64_TEXTURE_HEIGHT );

        // Print rom ptr, size and texture ptr
        printf("ROM pointer: %p, size: %zu bytes\n", (void*)rom, romSize);
        printf("Texture pointer: %p\n", (void*)texture);

        sm64_global_terminate();
        sm64_global_init(rom, texture);

        sm64_register_debug_print_function(debugPrint);

        initTestLevel();
        initMario();
        #endif
    }

    void free() {
        #ifdef ENABLE_MARIO
        sm64_global_terminate();

        if (texture) {
            ::free(texture);
            texture = nullptr;
        }
        if (rom) {
            ::free(rom);
            rom = nullptr;
            romSize = 0;
        }
        #endif
    }

    void update() {
        #ifdef ENABLE_MARIO

        // Get player entity
        if (GetAsyncKeyState(VK_NUMPAD6)) {
            auto playerPos = Halo1::getPlayerPosition();
            if (playerPos.has_value()) {
                auto pos = playerPos.value();
                // Convert Halo CE coordinates to Super Mario 64 coordinates
                auto marioPos = Coordinates::haloToMario(pos);
                sm64_set_mario_position(marioId, marioPos.x, marioPos.y, marioPos.z);
                sm64_mario_heal(marioId, 0xFF);
            }
        }
        
        // uint64_t currentTime = GetTickCount64();
        // #define MAYBE_PRESS(btn, prob) if (rand() % 100 < prob) { marioInputs.button##btn = 1; } else { marioInputs.button##btn = 0; }
        // MAYBE_PRESS(A, 5);
        // MAYBE_PRESS(B, 2);
        // MAYBE_PRESS(Z, 1);
        // #undef MAYBE_PRESS
        // static float xSign = 1.0f;
        // // Random chance to flip xSign
        // if (rand() % 100 < 10) {
        //     xSign *= -1.0f;
        // }
        // // Update Mario inputs based on keyboard state
        // marioInputs.stickX = sinf(currentTime / 3000.0f) * xSign; // Simulate left/right movement
        // marioInputs.stickY = cosf(currentTime / 3000.0f); // Simulate forward/backward movement

        Mario::updateXboxControls(marioInputs, marioState);
        
        sm64_mario_tick(marioId, &marioInputs, &marioState, &marioGeometry);
        sm64_set_mario_water_level(marioId, -999999.99f);

        auto playerRec = Halo1::getPlayerRecord();
        if (playerRec) {
            auto player = playerRec->entity();
            if (player) {
                // Update Halo player position to match Mario position
                auto marioHaloPos = Coordinates::marioToHalo(Vec3{
                    marioState.position[0],
                    marioState.position[1],
                    marioState.position[2]
                });
                player->pos = marioHaloPos;
            }
        }

        #endif
    }

    // Vec3 

    void debugRender() {
        #ifdef ENABLE_MARIO

        Vec3 marioFwd = Vec3{
            sinf(marioState.faceAngle),
            cosf(marioState.faceAngle),
            0.0f,
        };
        Overlay::ESP::drawLine(
            Coordinates::marioToHalo(Vec3{
                marioState.position[0],
                marioState.position[1],
                marioState.position[2]
            }),
            Coordinates::marioToHalo(Vec3{
                marioState.position[0],
                marioState.position[1],
                marioState.position[2]
            }) + marioFwd * 0.5f,
            IM_COL32(255, 0, 0, 255)
        );

        // Draw list
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        // Render Mario.
        for (int i = 0; i < marioGeometry.numTrianglesUsed; i++) {
            Vec3* pos = (Vec3*)&marioGeometry.position[i * 3 * 3];
            Vec3* color = (Vec3*)&marioGeometry.color[i * 3 * 3];

            // Render triangle wireframe
            for (int i = 0; i < 3; i++) {
                Vec3 p1 = pos[i];
                Vec3 p2 = pos[(i + 1) % 3];

                Vec3 haloP1 = Coordinates::marioToHalo(p1);
                Vec3 haloP2 = Coordinates::marioToHalo(p2);

                // Convert color
                ImU32 colorIm = IM_COL32(
                    static_cast<uint8_t>(color[i].x * 255),
                    static_cast<uint8_t>(color[i].y * 255),
                    static_cast<uint8_t>(color[i].z * 255),
                    255 // Full opacity
                );

                Overlay::ESP::drawLine(haloP1, haloP2, colorIm);
            }
        }

        // #define ENABLE_DEBUG_MARIO_GEOMETRY 1
        #ifdef ENABLE_DEBUG_MARIO_GEOMETRY
        // Render static surfaces.
        for (size_t i = 0; i < staticSurfacesCount; i++) {
            SM64Surface& surface = staticSurfaces[i];
            Vec3i* pos = reinterpret_cast<Vec3i*>(&surface.vertices[0][0]);

            Vec3 center = (pos[0].toVec3() + pos[1].toVec3() + pos[2].toVec3()) / 3.0f;
            Vec3 haloCenter = Coordinates::marioToHalo(center);

            Camera& camera = Overlay::ESP::camera;
            Vec3 toCenter = haloCenter - camera.pos;
            if (toCenter.length() > 60.0f) {
                continue; // Skip rendering if the surface is too far away
            }

            // Render triangle wireframe
            for (int i = 0; i < 3; i++) {
                Vec3 p1 = pos[i].toVec3();
                Vec3 p2 = pos[(i + 1) % 3].toVec3() ;

                Vec3 haloP1 = Coordinates::marioToHalo(p1);
                Vec3 haloP2 = Coordinates::marioToHalo(p2);

                ImU32 colorIm = IM_COL32(0, 255, 0, 40); // Green color for static surfaces

                Overlay::ESP::drawLine(haloP1, haloP2, colorIm);
            }

            Vec3 p0 = pos[0].toVec3();
            Vec3 p1 = pos[1].toVec3();
            Vec3 p2 = pos[2].toVec3();

            Vec3 haloP0 = Coordinates::marioToHalo(p0);
            Vec3 haloP1 = Coordinates::marioToHalo(p1);
            Vec3 haloP2 = Coordinates::marioToHalo(p2);

            Vec3 edge1 = haloP1 - haloP0;
            Vec3 edge2 = haloP2 - haloP0;
            Vec3 haloNormal = edge2.cross(edge1).normalize();

            // Draw surface normal
            ImU32 normalColor = IM_COL32(255, 0, 0, 160); // Red color for normal
            Overlay::ESP::drawCircle(haloCenter, 0.05f, normalColor, true);
            Overlay::ESP::drawLine(haloCenter, haloCenter + haloNormal * 0.5f, normalColor);

        }
        #endif // ENABLE_DEBUG_MARIO_GEOMETRY

        #endif // ENABLE_MARIO
    }
}