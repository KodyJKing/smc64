#include "Mario.hpp"

#include "libsm64.h"
#include "decomp/sm64.h"
#include "decomp/surface_terrains.h"
#include "decomp/audio_defines.h"

#include <cstdlib>
#include <cstdio>
#include <cstdint>
#include <cmath>
#include <string>
#include <iostream>
#include <mutex>

#include <Windows.h>

#include <filesystem>
#include "engine/halo1.hpp"
#include "MarioInput.hpp"
#include "BSPConversion.hpp"
#include "MarioBSPChunk.hpp"
#include "math/Vectors.hpp"
#include "Coordinates.hpp"
#include "DynamicGeometry.hpp"
#include "MarioFacing.hpp"
#include "MarioGameSpeed.hpp"
#include "MarioShieldRegen.hpp"

#include "spark/hook/Hooks.hpp"
#include "spark/overlay/Gizmos.hpp"
#include "imgui.h"

#include "MarioAudio.hpp"
#include "MarioCamera.hpp"
#include "ThirdPersonFix.hpp"
#include "MarioPickingFix.hpp"
#include "MarioDamageHook.hpp"
#include "MarioSkeleton.hpp"
#include "MarioModel.hpp"
#include "level-edit/MarioLevelEdit.hpp"
#include "MarioMelee.hpp"
#include "MarioWeaponOffset.hpp"
#include "MarioAimingIK.hpp"
#include "MarioChiefPose.hpp"
#include "MarioWeaponKick.hpp"
#include "AdrenalineSystem.hpp"
#include "MarioGoombaStomp.hpp"
#include "MarioShell.hpp"
#include "functions/MarioToCheif.hpp"
#include "functions/CheifToMario.hpp"
#include "functions/KillPlayer.hpp"
#include "functions/SanitizeAction.hpp"

// #define DEBUG_MARIO 1

#ifdef DEBUG_MARIO
    #define LOG(x) std::cout << "[Mario] " << x << std::endl;
#else
    #define LOG(x) ;
#endif

namespace Mod::Mario {

    // Guards the geometry buffers and full update() body against concurrent free().
    static std::mutex s_updateMutex;

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

    // libsm64 packs debug colors as 0xRRGGBBAA; ImGui's ImU32 is 0xAABBGGRR.
    static ImU32 marioColorToImU32(uint32_t colorRGBA) {
        uint8_t r = (colorRGBA >> 24) & 0xFF;
        uint8_t g = (colorRGBA >> 16) & 0xFF;
        uint8_t b = (colorRGBA >> 8)  & 0xFF;
        uint8_t a =  colorRGBA        & 0xFF;
        return IM_COL32(r, g, b, a);
    }

    // libsm64 debug callbacks report positions in Mario-local space (relative to
    // the active chunk). Convert to Halo world space before queueing gizmos.

    void debugLine(float x1, float y1, float z1, float x2, float y2, float z2, uint32_t colorRGBA, uint32_t durationFrames) {
        Vec3 start = Coordinates::marioLocalToHaloWorld(Vec3{ x1, y1, z1 }, marioChunk);
        Vec3 end   = Coordinates::marioLocalToHaloWorld(Vec3{ x2, y2, z2 }, marioChunk);
        Spark::Overlay::Gizmos::drawLine(start, end, marioColorToImU32(colorRGBA), durationFrames);
    }

    void debugPoint(float x, float y, float z, uint32_t colorRGBA, uint32_t durationFrames) {
        Vec3 pos = Coordinates::marioLocalToHaloWorld(Vec3{ x, y, z }, marioChunk);
        Spark::Overlay::Gizmos::drawPoint(pos, marioColorToImU32(colorRGBA), durationFrames);
    }

    void debugWorldText(float x, float y, float z, uint32_t colorRGBA, const char *text, uint32_t durationFrames) {
        Vec3 pos = Coordinates::marioLocalToHaloWorld(Vec3{ x, y, z }, marioChunk);
        Spark::Overlay::Gizmos::drawText(pos, text ? text : "", marioColorToImU32(colorRGBA), durationFrames);
    }

    void createSpawnPlatform(const Vec3& localPos) {
        // Create a temporary square floor in mario-local space to support spawn.
        // This is replaced with real BSP geometry immediately after by MarioBSPChunk::init.
        int32_t fx = (int32_t)std::lround(localPos.x);
        int32_t fz = (int32_t)std::lround(localPos.z);
        int32_t fy = (int32_t)std::lround(localPos.y) - 200; // ~0.5m below spawn
        int32_t half = 2000; // ~5m half-extent
        SM64Surface floor[2] = {};
        for (auto& s : floor) { s.type = SURFACE_DEFAULT; s.force = 0; s.terrain = TERRAIN_GRASS; }
        // CCW winding (viewed from above) for upward normal
        floor[0].vertices[0][0] = fx - half; floor[0].vertices[0][1] = fy; floor[0].vertices[0][2] = fz - half;
        floor[0].vertices[1][0] = fx - half; floor[0].vertices[1][1] = fy; floor[0].vertices[1][2] = fz + half;
        floor[0].vertices[2][0] = fx + half; floor[0].vertices[2][1] = fy; floor[0].vertices[2][2] = fz + half;
        floor[1].vertices[0][0] = fx - half; floor[1].vertices[0][1] = fy; floor[1].vertices[0][2] = fz - half;
        floor[1].vertices[1][0] = fx + half; floor[1].vertices[1][1] = fy; floor[1].vertices[1][2] = fz + half;
        floor[1].vertices[2][0] = fx + half; floor[1].vertices[2][1] = fy; floor[1].vertices[2][2] = fz - half;
        sm64_static_surfaces_load(floor, 2);
    }

    void initMario() {
        // Initialize Mario state.
        marioInputs = {};
        marioState = {};
        
        // Create a Mario instance at Cheif's position (split into chunk + local).
        if (marioId < 0) {

            // Caveat: Mario will not render until player has exited a vehicle (including after checkpoint reloads).
            if (Engine::isPlayerInputDisabled() || Engine::isPlayerInVehicle()) {
                printf("initMario: player input is disabled, skipping Mario spawn.\n");
                return;
            }
            
            auto playerPosOpt = Engine::getPlayerPosition();
            if (!playerPosOpt.has_value()) {
                printf("initMario: no player position available, skipping Mario spawn.\n");
                return;
            }
            Vec3 playerPos = playerPosOpt.value();

            marioTickCount = 0; // Reset Mario tick count when initializing Mario.

            Vec3 marioWorldPos = Coordinates::haloToMario(playerPos);
            marioChunk = Coordinates::marioChunkForPosition(marioWorldPos);
            Vec3 local = Coordinates::marioWorldToLocal(marioWorldPos, marioChunk);
            std::cout << "Local position: (" << local.x << ", " << local.y << ", " << local.z << ")\n";

            // Update Mario state's position so it's available before the first sm64 tick.
            marioState.position[0] = local.x;
            marioState.position[1] = local.y;
            marioState.position[2] = local.z;
            
            // Create temporary spawn platform, then create Mario.
            createSpawnPlatform(local);
            marioId = sm64_mario_create(local.x, local.y, local.z);

            // Initialize the last_mario_action global to Mario's initial action.
            uint32_t savedAction = Engine::Scripting::readGlobal("last_mario_action");
            uint32_t sanitizedAction = Mod::Mario::sanitizePersistedAction(savedAction);
            sm64_set_mario_action(marioId, sanitizedAction);
        }
        if (marioId < 0) {
            printf("Failed to create Mario instance.\n");
            Beep(750, 100);
            Beep(750, 100);
            return;
        }
        printf("Created Mario instance with ID: %d in chunk (%d, %d, %d)\n",
               marioId, marioChunk.x, marioChunk.y, marioChunk.z);

        // Initialize Mario geometry buffers.
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

        // Now we can load the level for Mario.
        sm64_set_mario_gas_level(marioId, -999999.99f);
        sm64_set_mario_water_level(marioId, -999999.99f);
        MarioBSPChunk::init(marioChunk);
    }

    void deinitMario() {
        LOG("deinitMario() called, deleting Mario instance.");
        if (marioId >= 0) {
            std::lock_guard<std::mutex> sm64Lock(MarioAudio::sm64Mutex());
            sm64_mario_delete(marioId);
            marioId = -1;
        }
        marioInputs = {};
        marioState = {};
        if (marioGeometry.position) { ::free(marioGeometry.position); marioGeometry.position = nullptr; }
        if (marioGeometry.color)    { ::free(marioGeometry.color);    marioGeometry.color    = nullptr; }
        if (marioGeometry.normal)   { ::free(marioGeometry.normal);   marioGeometry.normal   = nullptr; }
        if (marioGeometry.uv)       { ::free(marioGeometry.uv);       marioGeometry.uv       = nullptr; }
    }

    // Public:
    
    void init(Spark::ModId modId) {
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

        // sm64_register_debug_print_function(debugPrint);
        sm64_register_debug_line_function(debugLine);
        sm64_register_debug_point_function(debugPoint);
        sm64_register_debug_world_text_function(debugWorldText);

        MarioAudio::init(rom);

        ThirdPersonFix::registerHandlers(modId);
        MarioPickingFix::registerHandlers(modId);
        MarioDamageHook::registerHandlers(modId);
        MarioCamera::registerHandlers(modId);
        MarioWeaponOffset::registerHandlers(modId);
        MarioWeaponKick::registerHandlers(modId);
        GoombaStomp::init(modId);
        // AdrenalineSystem::registerHandlers(modId);
        #endif
    }

    void free() {
        #ifdef ENABLE_MARIO
        LOG("free() called, freeing Mario state and resources.");

        // Join the audio thread first — it holds s_sm64Mutex during sm64_audio_tick.
        // sm64_mario_delete and sm64_global_terminate don't acquire that mutex, so
        // calling them while the audio thread is running causes a race/crash on reinit.
        MarioAudio::free();

        DynamicGeometry::free();
        MarioBSPChunk::free();

        // Hold s_updateMutex while freeing state that update() reads.
        // This ensures any in-flight update() call completes before we pull
        // the buffers out from under it.
        std::lock_guard<std::mutex> updateLock(s_updateMutex);

        deinitMario();

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

    void dumpMarioGeometry();

    void deathTick() {
        if (marioId < 0 || !enableMario) return;

        sm64_mario_kill(marioId);

        std::lock_guard<std::mutex> sm64Lock(MarioAudio::sm64Mutex());

        LOG("Mario death tick start");
        sm64_mario_tick(marioId, &marioInputs, &marioState, &marioGeometry, marioBoneMatrices);
        LOG("Mario death tick end");

        updateMarioPose(marioBoneMatrices);
    }

    void uninitializedTick() {
        MarioChiefPose::updatePose();
    }

    void update() {
        #ifdef ENABLE_MARIO
        std::lock_guard<std::mutex> updateLock(s_updateMutex);

        if (Engine::isPlayerInputDisabled() || ::Mod::Mario::LevelEdit::isInputSuppressed()) {
            // Don't do anything with Mario if player input is disabled or the level editor has focus.
            MarioCamera::onDisable();
            if (Engine::isPlayerInputDisabled()) deinitMario();
            uninitializedTick();
            return;
        }

        if (enableMario && marioId < 0) {
            initMario();
            return;
        }

        auto playerRec = Engine::getPlayerRecord();
        if (!playerRec) return deathTick();
        auto player = playerRec->entity();
        if (!player) return deathTick();

        updateGameSpeed(*player);
        updateShieldRegen(player);
        // Engine::Scripting::submit("object_set_scale (player0) 0.4 1");

        MarioCamera::onUpdate(marioWorldPosition());

        if (GetAsyncKeyState(VK_F3) & 1) enableMario = !enableMario;
        if (GetAsyncKeyState(VK_F4) & 1) {
            possessMario = !possessMario;
            if (possessMario) marioToCheif();
        }

        if (GetAsyncKeyState(VK_NUMPAD1) & 1) {
            // sm64_set_mario_action_arg(marioId, ACT_RIDING_SHELL_GROUND, 0);
            // sm64_set_mario_action_arg(marioId, ACT_CRAZY_BOX_BOUNCE, 0);
            // sm64_set_mario_state(marioId, marioState.flags | MARIO_WING_CAP);
            // sm64_set_mario_action(marioId, ACT_FLYING_TRIPLE_JUMP);
            // sm64_set_mario_action(marioId, ACT_VERTICAL_WIND);
            // sm64_set_mario_velocity(marioId, 0, 150, 0);
            Mod::Mario::killPlayer();
            // sm64_set_mario_action(marioId, ACT_GROUND_BONK);
        }

        if (marioId < 0 || !enableMario) {
            MarioCamera::onDisable();
            return;
        }

        DynamicGeometry::update(marioState);

        if (marioInControl()) {
            // std::cout << "Mario in control this tick" << std::endl;
            cheifToMario(player);
            Mario::updateInput(marioInputs, marioState, Engine::getPlayerCameraPointer());
        } else {
            // std::cout << "Chief in control this tick" << std::endl;
            MarioCamera::onDisable();
            // Clear input state.
            marioInputs = {}; 
            marioToCheif();
        }
        
        faceLookDirection(Engine::getPlayerCameraPointer()->fwd);

        MarioBSPChunk::maintain();

        {
            std::lock_guard<std::mutex> sm64Lock(MarioAudio::sm64Mutex());
            if (marioInControl()) {
                LOG("Mario tick start");
                sm64_mario_tick(marioId, &marioInputs, &marioState, &marioGeometry, marioBoneMatrices);
                marioTickCount++; // Increment Mario tick count after each tick.
                LOG("Mario tick end");
            }
        }
        
        MarioAudio::update();

        updateMarioPose(marioBoneMatrices);
        if (marioInControl()) {
            MarioAimingIK::apply();
            MarioMelee::tick();
        } else {
            // MarioChiefPose::updatePose();
        }
        Mod::Mario::Shell::updateShellPose();

        // For now, always heal Mario. Cheif is responsible for recieving damage.
        sm64_mario_heal(marioId, 0xFF);

        Mod::Mario::Shell::updateShellState();

        // Save last_mario_action global.
        std::string command = "(set last_mario_action " + std::to_string(marioState.action) + ")";
        Engine::Scripting::submit(command.c_str());

        #endif
    }

}

