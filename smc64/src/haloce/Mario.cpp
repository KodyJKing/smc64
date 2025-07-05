#include "haloce/Mario.hpp"
#include "math/Vectors.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "overlay/ESP.hpp"

#include "libsm64.h"

#include <cstdlib>
#include <cstdio>
#include <cstdint>

#include <Windows.h>

#include <filesystem>
#include "Halo1.hpp"
#include "Mario.hpp"
#include "StaticGeometry.hpp"


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
            marioId = sm64_mario_create(0, 0, 0);
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
    }

    void free() {
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
    }

    void update() {

        // Check spacebar input
        if (GetAsyncKeyState(VK_SPACE) & 0x8000) {
            marioInputs.buttonA = 1;
        } else {
            marioInputs.buttonA = 0;
        }

        // Get player entity
        if (GetAsyncKeyState(VK_NUMPAD6)) {
            Halo1::EntityRecord* playerRecord = Halo1::getPlayerRecord();
            if (playerRecord) {
                Halo1::Entity* playerEntity = playerRecord->entity();
                if (playerEntity) {
                    auto pos = playerEntity->pos;
                    auto vel = playerEntity->vel;
                    
                    const float scaleFactor = 100.0f;
                    sm64_set_mario_position(marioId, pos.x * scaleFactor, pos.y * scaleFactor, pos.z * scaleFactor);
                }
            }
        }

        sm64_mario_tick(marioId, &marioInputs, &marioState, &marioGeometry);
    }

    void debugRender() {

        // Draw list
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        // Render Mario.
        for (int i = 0; i < marioGeometry.numTrianglesUsed; i++) {
            Vec3* pos = (Vec3*)&marioGeometry.position[i * 3 * 3];
            Vec3* color = (Vec3*)&marioGeometry.color[i * 3 * 3];

            const float scaleFactor = 100.0f;

            // Render triangle wireframe
            for (int i = 0; i < 3; i++) {
                Vec3 p1 = pos[i] / scaleFactor;
                Vec3 p2 = pos[(i + 1) % 3] / scaleFactor;

                // Convert color
                ImU32 colorIm = IM_COL32(
                    static_cast<uint8_t>(color[i].x * 255),
                    static_cast<uint8_t>(color[i].y * 255),
                    static_cast<uint8_t>(color[i].z * 255),
                    255 // Full opacity
                );

                Overlay::ESP::drawLine(p1, p2, colorIm);
            }
        }

    }
}