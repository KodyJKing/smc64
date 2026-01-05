#include "imgui.h"
#include "haloce/Mod.hpp"
#include "overlay/ESP.hpp"
#include "UI.hpp"
#include "haloce/Halo1.hpp"
#include "haloce/Mario.hpp"
#include "halomcc/HaloMCC.hpp"
#include "utils/ImGuiUtils.hpp"
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include <mutex>
#include <thread>
#include "utils/Utils.hpp"
#include "utils/Strings.hpp"
#include "memory/Memory.hpp"
#include "haloce/TimeScale.hpp"
#include "TagBrowser.hpp"
#include "Interpretations.hpp"
#include "cheatengine/Messages.hpp"

#include <Windows.h>

namespace HaloCE::Mod::UI
{

    void renderESP();
    void checkHotKeys();
    void settingsTab();
    void devTab();

    void interpretPointer(void *entity);

    bool showEsp = false;

    extern "C" __declspec(dllexport) Halo1::Entity *highlightEntity = nullptr;

    void topLevelRender()
    {
        checkHotKeys();

        if (showEsp)
        {
            Overlay::ESP::beginESPWindow("__ESP");
            renderESP();
            Overlay::ESP::endESPWindow();
        }
    }

    void checkHotKeys()
    {
        if (ImGui::IsKeyPressed(ImGuiKey_F1, false))
            showEsp = !showEsp;
        if (ImGui::IsKeyPressed(ImGuiKey_F2, false))
            HaloCE::Mod::settings.enableTimeScale = !HaloCE::Mod::settings.enableTimeScale;
        if (ImGui::IsKeyPressed(ImGuiKey_F3, false))
            HaloCE::Mod::settings.poseInterpolation = !HaloCE::Mod::settings.poseInterpolation;
    }

    void mainWindowTabs()
    {
        if (ImGui::BeginTabItem("Settings"))
        {
            settingsTab();
            ImGui::EndTabItem();
        }
        if (ImGui::BeginTabItem("Dev"))
        {
            devTab();
            ImGui::EndTabItem();
        }
    }

    void settingsTab()
    {
        ImGui::SeparatorText("Damage Scale");
        ImGui::SliderFloat("Player", &HaloCE::Mod::settings.playerDamageScale, 0.0f, 5.0f, "%.1f");
        ImGui::SliderFloat("NPC", &HaloCE::Mod::settings.npcDamageScale, 0.0f, 5.0f, "%.1f");
    }

    void devTab()
    {
        if (ImGui::CollapsingHeader("Time Scale"))
        {
            ImGui::Checkbox("Enable Time Scale", &HaloCE::Mod::settings.enableTimeScale);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Enable time scaling (F2)");

            ImGui::Checkbox("Override Time Scale", &HaloCE::Mod::settings.overrideTimeScale);
            int timeScalePercent = (int)(HaloCE::Mod::settings.timeScale * 100.0f);
            ImGui::PushItemWidth(200);
            ImGui::SliderInt("Scale", &timeScalePercent, 0, 100, "%d%%");
            ImGui::PopItemWidth();
            HaloCE::Mod::settings.timeScale = timeScalePercent / 100.0f;

            ImGui::Checkbox("Time Scale Deadzoning", &HaloCE::Mod::settings.timescaleDeadzoning);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Keep entities from updating at very low time scales.");

            ImGui::Checkbox("Pose Interpolation", &HaloCE::Mod::settings.poseInterpolation);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Enable pose interpolation (F3)");
        }

        if (ImGui::CollapsingHeader("Tools")) {
            // Translate map address
            ImGui::BeginChild("##Translate Map Address", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);

            auto renderAddressInput = [](const char *label, char *buffer, size_t bufferSize, const char *outputFormat, auto translateFunc)
            {
                ImGui::Text("%s", label);
                char labelId[255] = {0};
                snprintf(labelId, 255, "##%s", label);
                ImGui::InputText(labelId, buffer, bufferSize);
                uint64_t inputAddress = 0;
                try
                {
                    inputAddress = std::stoull(buffer, nullptr, 16);
                }
                catch (...)
                {
                    inputAddress = 0;
                }
                if (inputAddress)
                {
                    using ArgType = decltype(translateFunc);
                    uint64_t translated = 0;
                    if constexpr (std::is_invocable_v<decltype(translateFunc), uint32_t>)
                    {
                        translated = translateFunc(static_cast<uint32_t>(inputAddress));
                    }
                    else
                    {
                        translated = translateFunc(inputAddress);
                    }
                    ImGui::Text("-> %p", (void *)translated);
                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Right click to copy translated address to clipboard");
                    if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
                    {
                        char text[255] = {0};
                        snprintf(text, 255, outputFormat, (void *)translated);
                        ImGui::SetClipboardText(text);
                    }
                }
            };

            static char address[255] = {0};
            static char address2[255] = {0};
            renderAddressInput("From Map Relative Address", address, 255, "%p", Halo1::translateMapAddress);
            renderAddressInput("To Map Relative Address", address2, 255, "%X", Halo1::translateToMapAddress);

            ImGui::EndChild();

            ImGui::BeginChild("##Interpret U32", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
            static char u32input[255] = {0};
            ImGui::InputText("U32 Input", u32input, sizeof(u32input));
            uint32_t value = 0;
            try
            {
                value = std::stoul(u32input, nullptr, 16);
            }
            catch (...)
            {
                value = 0;
            }
            if (value)
            {
                ImGui::Text("Interpretations of %X:", value);
                ImGui::Separator();
                interpretations(value);
            }
            ImGui::EndChild();

            ImGui::BeginChild("##Interpret Object Fields", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
            static char addressInput[255] = {0};
            ImGui::InputText("Address", addressInput, sizeof(addressInput));
            uintptr_t pointerValue = 0;
            try
            {
                pointerValue = std::stoull(addressInput, nullptr, 16);
            }
            catch (...)
            {
                pointerValue = 0;
            }
            Halo1::Entity *entity = (Halo1::Entity *)pointerValue;
            if (entity && Memory::isAllocated((uintptr_t)entity))
            {
                ImGui::Text("Interpreting object fields %p", (void *)entity);
                ImGui::Separator();

                ImGui::Begin("Object Field Interpretations");
                {
                    interpretPointer(entity);
                }
                ImGui::End();

                // highlightEntity = entity; // For ESP highlighting.
            }
            else
            {
                ImGui::Text("Entity not found for handle %X", pointerValue);
            }
            ImGui::EndChild();

            if (ImGui::Button("Tag Browser"))
                showTagBrowser = !showTagBrowser;
            if (showTagBrowser)
                tagBrowser();

            ImGui::Checkbox("ESP", &showEsp);
            if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Toggle ESP (F1)");
        }

        // if (ImGui::Button("Test IPC")) {
        //     CE::Messages::openHexView(0x12345678);
        // }
    }

    /////////////////////////////////////////////////////////////////////////////////////////////
    // ESP
    // Todo: Move into Halo1ESP.cpp

    struct ESPSettings
    {
        bool anchorHighlight = false;
        float fovScale = 0.627f;
        float maxDistance = 100.0f;
        float maxBSPVertexDistance = 20.0f;
        struct Filter
        {
            bool biped = true;
            bool vehicle = false;
            bool weapon = false;
            bool projectile = false;
            bool scenery = false;
            bool equipment = false;
            bool other = false;
        } filter = {};
    } espSettings = {};

    bool shouldDisplay(Halo1::Entity *entity)
    {
        auto c = (Halo1::EntityCategory)entity->entityCategory;
        if (c == Halo1::EntityCategory_Biped)
            return espSettings.filter.biped;
        if (c == Halo1::EntityCategory_Vehicle)
            return espSettings.filter.vehicle;
        if (c == Halo1::EntityCategory_Weapon)
            return espSettings.filter.weapon;
        if (c == Halo1::EntityCategory_Projectile)
            return espSettings.filter.projectile;
        if (c == Halo1::EntityCategory_Scenery)
            return espSettings.filter.scenery;
        if (c == Halo1::EntityCategory_Equipment)
            return espSettings.filter.equipment;
        return espSettings.filter.other;
    }

    void filterSettings()
    {
        ImGui::Checkbox("Biped", &espSettings.filter.biped);
        ImGui::Checkbox("Vehicle", &espSettings.filter.vehicle);
        ImGui::Checkbox("Weapon", &espSettings.filter.weapon);
        ImGui::Checkbox("Projectile", &espSettings.filter.projectile);
        ImGui::Checkbox("Scenery", &espSettings.filter.scenery);
        ImGui::Checkbox("Equipment", &espSettings.filter.equipment);
        ImGui::Checkbox("Other", &espSettings.filter.other);
    }

    // Halo1::Entity* highlightEntity = nullptr;

    void highlightedEntityDetails()
    {
        Halo1::Entity *entity = highlightEntity;

        if (entity == nullptr || !Memory::isAllocated((uintptr_t)entity))
            return;

        static struct View
        {
            bool pos = true;
            bool vel;
            bool health;
            bool shield;
            bool tag;
            bool tagID;
            bool tagCC;
            bool tagPath = true;
            bool animation;
            bool bones;
            bool collision;
        } view = {};

        bool paused = HaloMCC::isPauseMenuOpen();

#define VIEW_TOGGLE(name)                        \
    if (paused)                                  \
    {                                            \
        ImGui::Checkbox("##" #name, &view.name); \
        ImGui::SameLine();                       \
    }

        auto tag = entity->tag();
        if ((!paused || ImGui::CollapsingHeader("Tag")) && tag)
        {
            VIEW_TOGGLE(tag);
            if (paused || view.tag)
                ImGui::Text("Tag: %p", tag);

            VIEW_TOGGLE(tagID);
            if (paused || view.tagID)
                ImGui::Text("Tag ID: %X", entity->tagID);

            std::string groupID = tag->groupIDStr();
            VIEW_TOGGLE(tagCC);
            if (paused || view.tagCC)
                ImGui::Text("Tag GroupID: %s", groupID.c_str());

            VIEW_TOGGLE(tagPath);
            const char *path = tag->getResourcePath();
            const char *nullPath = "null";
            if (!path)
                path = nullPath;
            if (paused || view.tagPath)
                ImGui::Text("Tag Path: %s", path);
        }

        if (!paused || ImGui::CollapsingHeader("Entity"))
        {
            auto pos = entity->pos;
            auto vel = entity->vel;
            VIEW_TOGGLE(pos);
            if (paused || view.pos)
                ImGui::Text("Position: %.2f, %.2f, %.2f", pos.x, pos.y, pos.z);
            VIEW_TOGGLE(vel);
            if (paused || view.vel)
                ImGui::Text("Velocity: %.2f, %.2f, %.2f", vel.x, vel.y, vel.z);

            VIEW_TOGGLE(health);
            if (paused || view.health)
                ImGui::Text("Health: %.2f", entity->health);
            VIEW_TOGGLE(shield);
            if (paused || view.shield)
                ImGui::Text("Shield: %.2f", entity->shield);

            VIEW_TOGGLE(animation);
            if (paused || view.animation)
                ImGui::Text("AnimSet: %X, Anim: %d, Frame: %d, %d bones", entity->animSetTagID, entity->animId, entity->animFrame, entity->boneCount());

            VIEW_TOGGLE(bones);
            if (paused || view.bones) {
                auto boneTransforms = entity->getBoneTransforms();
                ImGui::Text("Bones: %p", boneTransforms);
                if (view.bones && boneTransforms)
                {
                    ImGui::BeginChild("Bones", ImVec2(0, 0), ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY | ImGuiChildFlags_AutoResizeX);
                    for (int i = 0; i < entity->boneCount(); i++)
                    {
                        auto bone = boneTransforms[i];
                        auto pos = bone.translation;
                        auto rot = bone.rotation;
                        ImGui::Text("%02d {%.2f, %.2f, %.2f, %.2f} {%.2f, %.2f, %.2f}", i, rot.x, rot.y, rot.z, rot.w, pos.x, pos.y, pos.z);
                    }
                    ImGui::EndChild();
                }
            }

            VIEW_TOGGLE(collision);
            if (paused || view.collision) {

                ImGui::BeginChild("Collision Geometry", ImVec2(0, 0),  ImGuiChildFlags_AutoResizeY);
                {
                    auto collisionTagId = Halo1::collisionGeometryTagId(tag);
                    ImGui::Text("Collision Tag ID: %X", collisionTagId);
                    ImGui::Indent(20.0f);
                    if (view.collision && collisionTagId != NULL_HANDLE) {
                        auto collisionTag = Halo1::getTag(collisionTagId);
                        if (collisionTag) {
                            auto collisionPath = collisionTag->getResourcePath();
                            ImGui::Text("Tag Path: %s", collisionPath ? collisionPath : "null");

                            auto collisionData = collisionTag->getData();
                            char buffer[256];
                            snprintf(buffer, sizeof(buffer), "%p", collisionData);
                            ImGuiUtils::renderCopyableText("Tag Data:", buffer);
                        } else {
                            ImGui::Text("Tag not found");
                        }
                    }
                    ImGui::Unindent(20.0f);
                }
                ImGui::EndChild();


            }
        }
    }

    void espWindow()
    {
        bool paused = HaloMCC::isPauseMenuOpen();
        ImGuiWindowFlags flags = ImGuiWindowFlags_AlwaysAutoResize;
        if (!paused)
            flags |= ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoMouseInputs;

        ImGui::Begin("ESP", 0, flags);

        // Tabs
        if (ImGui::BeginTabBar("ESP Tabs"))
        {

            if (ImGui::BeginTabItem("Entities")) {
                ImGui::Text("%p", highlightEntity);

                // Copy button
                ImGui::SameLine();
                bool copyHotkeyDown = ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(ImGuiKey_C, false);
                if (copyHotkeyDown)
                    ImGui::PushStyleColor(ImGuiCol_Button, IM_COL32(0, 255, 0, 255));
                if (ImGui::Button("copy") || copyHotkeyDown) {
                    char text[255] = {0};
                    snprintf(text, 255, "%p", highlightEntity);
                    ImGui::SetClipboardText(text);
                }
                if (ImGui::IsItemHovered())
                ImGui::SetTooltip("Copy pointer to clipboard (Shift+C)");
                if (copyHotkeyDown)
                ImGui::PopStyleColor();
                
                // Browse button
                ImGui::SameLine();
                if (ImGui::Button("browse")) {
                    CE::Messages::openHexView((uintptr_t)highlightEntity);
                }

                ImGui::SameLine();
                ImGui::Checkbox("anchor", &espSettings.anchorHighlight);
                if (ImGui::IsItemHovered())
                    ImGui::SetTooltip("Keep current entity highlighted (Shift+V)");
                bool anchorHotkeyDown = ImGui::GetIO().KeyShift && ImGui::IsKeyPressed(ImGuiKey_V, false);
                if (anchorHotkeyDown)
                    espSettings.anchorHighlight = !espSettings.anchorHighlight;

                if (paused && ImGui::CollapsingHeader("Filter"))
                {
                    int maxDistInt = (int)espSettings.maxDistance;
                    ImGui::SliderInt("Max Distance", &maxDistInt, 1, 200, "%d");
                    espSettings.maxDistance = (float)maxDistInt;
                    filterSettings();
                }

                highlightedEntityDetails();

                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Camera"))
            {
                if (paused)
                    ImGui::SliderFloat("Fov Adjust", &espSettings.fovScale, 0.0f, 2.0f);
                Camera &camera = Overlay::ESP::camera;
                ImGui::Text("Pos: %.2f %.2f %.2f", camera.pos.x, camera.pos.y, camera.pos.z);
                ImGui::Text("Fwd: %.2f %.2f %.2f", camera.fwd.x, camera.fwd.y, camera.fwd.z);
                // ImGui::Text( "Up: %.2f %.2f %.2f", camera.up.x, camera.up.y, camera.up.z );
                ImGui::Text("Fov: %.2f", camera.fov);
                // ImGui::Text("Screen: %.2f x %.2f", camera.width, camera.height);
                ImGui::EndTabItem();
            }

#define PLAYER_CONTROLLER_TAB
#ifdef PLAYER_CONTROLLER_TAB
            if (ImGui::BeginTabItem("PlayerController"))
            {
                auto playerController = Halo1::getPlayerControllerPointer();
                ImGui::Text("%p", playerController);

                uint32_t actions = playerController->actions;
                for (int i = 0; i < 32; i++)
                {
                    if (i > 0 && i % 8 != 0)
                        ImGui::SameLine();
                    char text[255] = {0};
                    snprintf(text, 255, "##flag%d", i);
                    ImGui::CheckboxFlags(text, &actions, 1 << i);
                }

                ImVec4 green = ImVec4(0.0f, 1.0f, 0.0f, 1.0f);
                ImVec4 white = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
#define SHOW_ACTION_1(name) ImGui::TextColored((actions & Halo1::PlayerActionFlags::name) ? green : white, #name);
#define SHOW_ACTION_2(name) \
    ImGui::SameLine();      \
    SHOW_ACTION_1(name);
                SHOW_ACTION_1(crouch);
                SHOW_ACTION_2(jump);
                SHOW_ACTION_2(flash);
                SHOW_ACTION_2(melee);
                SHOW_ACTION_1(reload);
                SHOW_ACTION_2(shoot);
                SHOW_ACTION_2(grenade1);
                SHOW_ACTION_2(grenade2);
#undef SHOW_ACTION_2
#undef SHOW_ACTION_1

                ImGui::Text("WalkX %.2f", playerController->walkX);
                ImGui::Text("WalkY %.2f", playerController->walkY);

                ImGui::EndTabItem();
            }
#endif

// BSP Data
#define BSP_TAB
#ifdef BSP_TAB
            if (ImGui::BeginTabItem("BSP"))
            {
                uintptr_t bspPointer = Halo1::getBSPPointer();
                char bspPointerStr[255] = {0};
                snprintf(bspPointerStr, 255, "%p", (void *)bspPointer);
                ImGuiUtils::renderCopyableText("BSP Pointer", bspPointerStr);

                uint32_t bspVertexCount = Halo1::getBSPVertexCount();
                ImGui::Text("BSP Vertices: %d", bspVertexCount);

                Halo1::BSPVertex *bspVertices = Halo1::getBSPVertexArray();
                char bspVerticesStr[255] = {0};
                snprintf(bspVerticesStr, 255, "%p", bspVertices);
                ImGuiUtils::renderCopyableText("BSP Vertex Array", bspVerticesStr);

                // Edge count
                uint32_t bspEdgeCount = Halo1::getBSPEdgeCount();
                ImGui::Text("BSP Edges: %d", bspEdgeCount);

                // Edge array
                Halo1::BSPEdge *bspEdges = Halo1::getBSPEdgeArray();
                char bspEdgesStr[255] = {0};
                snprintf(bspEdgesStr, 255, "%p", bspEdges);
                ImGuiUtils::renderCopyableText("BSP Edge Array", bspEdgesStr);

                ImGui::EndTabItem();
            }
#endif

            ImGui::EndTabBar();
        }

        ImGui::End();
    }

    Vec3 displayPos(Halo1::Entity *entity)
    {
        return entity->rootBonePos;
        // if (
        //     // entity->entityCategory == Halo1::EntityCategory_Scenery ||
        //     entity->parentHandle != NULL_HANDLE
        // )
        //     return entity->rootBonePos;
        // return entity->pos;
    }

    void renderESP_entities()
    {
        namespace ESP = Overlay::ESP;
        Camera &camera = ESP::camera;

        std::vector<Halo1::Entity *> entitiesToDraw;
        Halo1::foreachEntityRecord([&](Halo1::EntityRecord *rec)
                                   {
            auto entity = rec->entity();
            if (shouldDisplay( entity )) {
                Vec3 pos = displayPos( entity );
                auto toEntity = pos - camera.pos;
                if (toEntity.length() < espSettings.maxDistance)
                    entitiesToDraw.push_back( entity );
            } });

        if (!espSettings.anchorHighlight)
        {
            int highlightIndex = -1;
            float maxDot = -1.0f;
            for (int i = 0; i < entitiesToDraw.size(); i++)
            {
                auto entity = entitiesToDraw[i];
                Vec3 pos = displayPos(entity);
                auto toEntity = pos - camera.pos;
                if (toEntity.length() > espSettings.maxDistance)
                    continue;
                toEntity = toEntity.normalize();
                float dot = toEntity.dot(camera.fwd);
                if (dot > maxDot)
                {
                    maxDot = dot;
                    highlightIndex = i;
                }
            }
            highlightEntity = highlightIndex >= 0 ? entitiesToDraw[highlightIndex] : nullptr;
        }

        bool gamePaused = HaloMCC::isPauseMenuOpen();
        byte alpha = gamePaused ? 0x40 : 0xFF;
        for (Halo1::Entity *entity : entitiesToDraw)
        {
            auto color = IM_COL32(255, 255, 255, alpha);
            float radius = 0.1f;
            if (entity == highlightEntity)
            {
                color = IM_COL32(64, 64, 255, alpha);
                radius = 0.2f;

                // Draw coordinate frame on highlighted entity
                auto red = IM_COL32(255, 64, 64, alpha);
                auto green = IM_COL32(64, 255, 64, alpha);
                auto blue = IM_COL32(64, 64, 255, alpha);
                Vec3 pos = displayPos(entity);
                Vec3 ex = Vec3{1, 0, 0} * 0.5f;
                Vec3 ey = Vec3{0, 1, 0} * 0.5f;
                Vec3 ez = Vec3{0, 0, 1} * 0.5f;
                ESP::drawLine(pos, pos + ex, red);
                ESP::drawLine(pos, pos + ey, green);
                ESP::drawLine(pos, pos + ez, blue);

                float nudgeSpeed = 0.1f;
                if (ImGui::GetIO().KeyShift)
                    nudgeSpeed *= 0.5f;

                // Numpad 4,6 to nudge -x,+x
                if (ImGui::IsKeyPressed(ImGuiKey_Keypad4, false))
                    entity->pos.x -= nudgeSpeed;
                if (ImGui::IsKeyPressed(ImGuiKey_Keypad6, false))
                    entity->pos.x += nudgeSpeed;
                // Numpad 8,5 to nudge +y,-y
                if (ImGui::IsKeyPressed(ImGuiKey_Keypad8, false))
                    entity->pos.y += nudgeSpeed;
                if (ImGui::IsKeyPressed(ImGuiKey_Keypad5, false))
                    entity->pos.y -= nudgeSpeed;
                // Numpad 7,9 to nudge +z,-z
                if (ImGui::IsKeyPressed(ImGuiKey_Keypad7, false))
                    entity->pos.z += nudgeSpeed;
                if (ImGui::IsKeyPressed(ImGuiKey_Keypad9, false))
                    entity->pos.z -= nudgeSpeed;
            }
            Vec3 pos = displayPos(entity);
            ESP::drawCircle(pos, radius, color, true);
        }
    }

    void renderESP_BSP()
    {
        namespace ESP = Overlay::ESP;
        Camera &camera = ESP::camera;

        uint32_t bspVertexCount = Halo1::getBSPVertexCount();
        Halo1::BSPVertex *bspVertices = Halo1::getBSPVertexArray();
        if (bspVertices == nullptr || bspVertexCount == 0)
            return;

        uint32_t bspEdgeCount = Halo1::getBSPEdgeCount();
        Halo1::BSPEdge *bspEdges = Halo1::getBSPEdgeArray();
        if (bspEdges == nullptr || bspEdgeCount == 0)
            return;

        uint32_t bspSurfaceCount = Halo1::getBSPSurfaceCount();
        Halo1::BSPSurface *bspSurfaces = Halo1::getBSPSurfaceArray();
        if (bspSurfaces == nullptr || bspSurfaceCount == 0)
            return;

        bool gamePaused = HaloMCC::isPauseMenuOpen();

        //// Draw BSP vertices

        byte alpha = gamePaused ? 0x40 : 0xFF;
        auto color = IM_COL32(255, 255, 0, alpha);
        float radius = 0.05f;

        for (uint32_t i = 0; i < bspVertexCount; i++)
        {
            auto vertex = &bspVertices[i];
            Vec3 pos = vertex->pos;
            auto toVertex = pos - camera.pos;
            if (toVertex.length() > espSettings.maxBSPVertexDistance)
                continue;
            ESP::drawPoint(pos, color);
        }

        //// Draw BSP surfaces

        alpha = gamePaused ? 0x20 : 0x40;
        color = IM_COL32(0, 255, 0, alpha);

        for (uint32_t i = 0; i < bspEdgeCount; i++)
        {
            auto edge = &bspEdges[i];

            auto p0 = &bspVertices[edge->startVertex];
            auto p1 = &bspVertices[edge->endVertex];

            // Bail early if p0 or p1 are invalid.
            auto toP0 = p0->pos - camera.pos;
            auto toP1 = p1->pos - camera.pos;
            if (toP0.length() > espSettings.maxBSPVertexDistance ||
                toP1.length() > espSettings.maxBSPVertexDistance)
                continue; // Skip edges that are too far away.

            uint32_t surfaces[2] = {edge->leftSurface, edge->rightSurface};
            for (uint32_t j = 0; j < 2; j++)
            {
                if (surfaces[j] >= bspSurfaceCount)
                    continue; // Invalid surface, skip it.
                auto surface = &bspSurfaces[surfaces[j]];
                // Get the first vertex of the first edge of the surface.
                if (surface->firstEdgeIndex >= bspEdgeCount)
                    continue; // Invalid surface, skip it.

                auto firstEdge = &bspEdges[surface->firstEdgeIndex];
                if (firstEdge->startVertex >= bspVertexCount)
                    continue; // Invalid edge, skip it.
                auto firstVertex = &bspVertices[firstEdge->startVertex];

                // // Draw triangle formed between edge and firstVertex, if it's not degenerate.
                if (edge->startVertex == firstEdge->startVertex ||
                    edge->endVertex == firstEdge->startVertex)
                {
                    // Degenerate, skip.
                    continue;
                }

                auto p2 = firstVertex;

                auto toP2 = p2->pos - camera.pos;
                if (toP2.length() > espSettings.maxBSPVertexDistance)
                    continue; // Skip triangles that are too far away.

                // Draw the triangle.
                ESP::drawLine(p0->pos, p1->pos, color);
                ESP::drawLine(p1->pos, p2->pos, color);
                ESP::drawLine(p2->pos, p0->pos, color);

                // // Render text for surface material.
                // Vec3 textPos = (p0->pos + p1->pos + p2->pos) / 3.0f;
                // auto material = surface->material;
                // char materialText[255] = {0};
                // sprintf( materialText, "%X", material );
                // ESP::drawText( textPos, materialText, color );
            }
        }
    }

    void renderESP()
    {
        if (!Halo1::isGameLoaded())
            return;

        espWindow();

        // Setup ESP camera to match player camera.
        Camera &camera = Overlay::ESP::camera;
        auto haloCam = Halo1::getPlayerCameraPointer();
        camera.pos = haloCam->pos;
        camera.fwd = haloCam->fwd;
        camera.up = haloCam->up;
        camera.fov = haloCam->fov * espSettings.fovScale;
        camera.verticalFov = true;

        renderESP_entities();

        // renderESP_BSP();

        // Mario debugRender
        HaloCE::Mod::Mario::debugRender();
    }

}