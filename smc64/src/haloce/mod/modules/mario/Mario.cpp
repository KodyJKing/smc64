#include "Mario.hpp"

#define IMGUI_DEFINE_MATH_OPERATORS
#include "imgui.h"
#include "overlay/ESP.hpp"

#include "libsm64.h"
#include "decomp/sm64.h"
#include "decomp/surface_terrains.h"

#include <cstdlib>
#include <cstdio>
#include <cstdint>

#include <Windows.h>

#include <filesystem>
#include "../../../halo1/Halo1.hpp"
#include "Mario.hpp"
#include "MarioInput.hpp"
#include "BSPConversion.hpp"
#include "math/Vectors.hpp"
#include "Coordinates.hpp"
#include "DynamicGeometry.hpp"

#include "../FreeCam.hpp"
#include "ThirdPersonFix.hpp"
#include "MarioSkeleton.hpp"

#define ENABLE_MARIO 1

namespace HaloCE::Mod::Mario {

    // Internal:
    bool enableMario = true;
    bool possessMario = true;

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
            auto id = sm64_mario_create(99999.0f, 99999.0f, 99999.0f);
            auto playerPos = Halo1::getPlayerPosition();
            if (playerPos.has_value()) {
                auto pos = playerPos.value();
                // Convert Halo CE coordinates to Super Mario 64 coordinates
                auto marioPos = Coordinates::haloToMario(pos);
                sm64_set_mario_position(id, marioPos.x, marioPos.y, marioPos.z);
            }
            marioId = id;
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
        // Load Halo CE static geometry and convert it to Super Mario 64 format
        auto surfaceVector = HaloCE::Mod::BSPConversion::haloGeometryToMario();

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

        ThirdPersonFix::init(Halo1::dllBase());
        #endif
    }

    void free() {
        #ifdef ENABLE_MARIO
        ThirdPersonFix::free();

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

    void marioToCheif() {
        auto playerPos = Halo1::getPlayerPosition();
        if (playerPos.has_value()) {
            auto pos = playerPos.value();
            // Convert Halo CE coordinates to Super Mario 64 coordinates
            auto marioPos = Coordinates::haloToMario(pos);
            sm64_set_mario_position(marioId, marioPos.x, marioPos.y, marioPos.z);
            sm64_mario_heal(marioId, 0xFF);
        }
    }

    void dumpMarioGeometry();

    void update() {
        #ifdef ENABLE_MARIO

        // F6 to dump Mario geometry buffers
        if (GetAsyncKeyState(VK_F6) & 1) {
            dumpMarioGeometry();
        }

        if (GetAsyncKeyState(VK_F3) & 1) enableMario = !enableMario;
        if (GetAsyncKeyState(VK_F4) & 1) {
            possessMario = !possessMario;
            if (possessMario) marioToCheif();
        }

        if (marioId < 0 || !enableMario) {
            Freecam::cameraOverride.enablePosition = false;
            return;
        }

        DynamicGeometry::update(marioState);

        if (possessMario) {
            Mario::updateInput(marioInputs, marioState, Halo1::getPlayerCameraPointer());
            auto playerRec = Halo1::getPlayerRecord();
            if (playerRec) {
                auto player = playerRec->entity();
                if (player) {

                    Vec3 marioWorldPos = Coordinates::marioToHalo(Vec3{
                        marioState.position[0],
                        marioState.position[1],
                        marioState.position[2]
                    });
                    // Vec3 difference = marioWorldPos - player->pos;
                    Vec3 difference = player->pos - marioWorldPos;
                    float distance = difference.length();
                    
                    // If Cheif teleported, move Mario to Cheif
                    if (distance > 5.0f) {
                        marioToCheif();
                    } else {
                        // Cheif to Mario

                        // Allow some vertical difference, to keep Cheif grounded.
                        float dz = difference.z;
                        float limit = 0.2f;
                        if (abs(dz) > limit) {
                            dz = (dz > 0) ? limit : -limit;
                        }

                        player->pos = marioWorldPos + Vec3{0, 0, dz};
                    }
                }
            }
        } else {
            Freecam::cameraOverride.enablePosition = false;
        }
        
        sm64_mario_tick(marioId, &marioInputs, &marioState, &marioGeometry);
        sm64_set_mario_water_level(marioId, -999999.99f);

        bool inForbiddenState = false
            || (marioState.action == ACT_START_SLEEPING)
            || (marioState.action == ACT_SLEEPING)
            || (marioState.action == ACT_WAKING_UP);
        if (inForbiddenState) sm64_set_mario_action(marioId, ACT_IDLE);

        #endif
    }

    // Vec3 

    void draw_SM64SurfaceCollisionData(SM64SurfaceCollisionData* surfaceData, ImU32 color) {
        namespace ESP = Overlay::ESP;
        Camera &camera = ESP::camera;
        Vec3 v1 = Coordinates::marioToHalo(surfaceData->vertex1);
        Vec3 v2 = Coordinates::marioToHalo(surfaceData->vertex2);
        Vec3 v3 = Coordinates::marioToHalo(surfaceData->vertex3);
        ESP::drawLine(v1, v2, color);
        ESP::drawLine(v2, v3, color);
        ESP::drawLine(v3, v1, color);

        Vec3 center = (v1 + v2 + v3) / 3.0f;
        auto n = surfaceData->normal;
        Vec3 normalEnd = center + Vec3{n.x, n.z, n.y} * 0.1f;
        ESP::drawLine(center, normalEnd, color);
        ESP::drawPoint(center, color);
    }

    int32_t highlightTriangleIndex = -1;

    void marioDebugWindow(SM64WallCollisionData& wallData) {
        ImGui::Begin("Mario Debug Info", nullptr, ImGuiWindowFlags_AlwaysAutoResize);
        
        // ImGui::Text("Mario ID: %d", marioId);
        // ImGui::Text("Position: (%.2f, %.2f, %.2f)", marioState.position[0], marioState.position[1], marioState.position[2]);
        // ImGui::Text("Velocity: (%.2f, %.2f, %.2f)", marioState.velocity[0], marioState.velocity[1], marioState.velocity[2]);
        // ImGui::Text("Health: %d", marioState.health);
        // ImGui::Text("Action: 0x%X", marioState.action);
        // ImGui::Text("Flags: 0x%X", marioState.flags);
        // ImGui::Text("Num walls: %d", wallData.numWalls);

        // Slider for highlightTriangleIndex
        ImGui::SliderInt("##Highlight Triangle Index", &highlightTriangleIndex, -1, marioGeometry.numTrianglesUsed - 1);

        ImGui::End();
    }

    void debugRender() {
        #ifdef ENABLE_MARIO

        // Draw list
        ImDrawList* drawList = ImGui::GetWindowDrawList();
        ImVec2 windowPos = ImGui::GetWindowPos();
        ImVec2 windowSize = ImGui::GetWindowSize();

        bool highlightEnabled = highlightTriangleIndex >= 0 && highlightTriangleIndex < static_cast<int32_t>(marioGeometry.numTrianglesUsed);

        // Render Mario.
        for (int i = 0; i < marioGeometry.numTrianglesUsed; i++) {
            Vec3* pos = (Vec3*)&marioGeometry.position[i * 3 * 3];
            Vec3* color = (Vec3*)&marioGeometry.color[i * 3 * 3];

            bool highlightThis = highlightEnabled && i == highlightTriangleIndex;
            // uint8_t alpha = !highlightEnabled || highlightThis ? 255 : 20;
            uint8_t alpha = 20;

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
                    alpha
                );

                Overlay::ESP::drawLine(haloP1, haloP2, colorIm);

            }

            if (highlightThis) {
                Vec3 haloP1 = Coordinates::marioToHalo(pos[0]);
                Vec3 haloP2 = Coordinates::marioToHalo(pos[1]);
                Vec3 haloP3 = Coordinates::marioToHalo(pos[2]);
                Overlay::ESP::drawPoint(haloP1, IM_COL32(255, 0, 0, 255));
                Overlay::ESP::drawPoint(haloP2, IM_COL32(0, 255, 0, 255));
                Overlay::ESP::drawPoint(haloP3, IM_COL32(0, 0, 255, 255));
            }
            
        }

        // SM64SurfaceCollisionData* floorData;
        // sm64_surface_find_floor(
        //     marioState.position[0],
        //     marioState.position[1],
        //     marioState.position[2],
        //     &floorData
        // );
        // if (floorData) 
        //     draw_SM64SurfaceCollisionData(floorData, IM_COL32(255, 200, 0, 255));

        // SM64SurfaceCollisionData* ceilData;
        // sm64_surface_find_ceil(
        //     marioState.position[0],
        //     marioState.position[1],
        //     marioState.position[2],
        //     &ceilData
        // );
        // if (ceilData) 
        //     draw_SM64SurfaceCollisionData(ceilData, IM_COL32(255, 0, 200, 255));
        
        // SM64WallCollisionData wallData = {};
        // wallData.x = marioState.position[0];
        // wallData.y = marioState.position[1];
        // wallData.z = marioState.position[2];
        // wallData.radius = 100.0f; // Large radius to capture nearby walls
        // wallData.offsetY = 10.0f;
        // sm64_surface_find_wall_collisions(&wallData);

        // for (int i = 0; i < wallData.numWalls; i++) {
        //     SM64SurfaceCollisionData* wallSurface = wallData.walls[i];
        //     if (wallSurface)
        //         draw_SM64SurfaceCollisionData(wallSurface, IM_COL32(0, 200, 255, 255));
        // }

        drawMarioBones(marioGeometry);

        // marioDebugWindow(wallData);

        #endif // ENABLE_MARIO
    }
}
