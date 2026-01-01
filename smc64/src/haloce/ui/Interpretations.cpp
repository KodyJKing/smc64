#include "Interpretations.hpp"
#include "imgui.h"
#include "haloce/Halo1.hpp"
#include "memory/Memory.hpp"
#include "utils/Strings.hpp"

namespace HaloCE::Mod::UI {
    void interpretations(uint32_t value) {
        auto result = Halo1::interpretU32(value);
        if (result.mapPointer && Memory::isAllocated((uintptr_t)result.mapPointer))
        {
            ImGui::Text("Map Pointer: %p", (void *)result.mapPointer);
            if (ImGui::CollapsingHeader("Data..."))
            {
                ImGui::Indent(20.0f);
                interpretPointer((void *)result.mapPointer);
                ImGui::Unindent(20.0f);
            }
        }
        if (result.tag && Memory::isAllocated((uintptr_t)result.tag))
        {
            ImGui::Separator();
            ImGui::Text("Tag: %p", (void *)result.tag);
            ImGui::Text("Tag Path: %s", result.tag->getResourcePath());
            auto groupID = result.tag->groupIDStr();
            ImGui::Text("Tag GroupID: %s", groupID.c_str());
            if (ImGui::CollapsingHeader("Tag Data..."))
            {
                ImGui::Indent(20.0f);
                interpretPointer(result.tag->getData());
                ImGui::Unindent(20.0f);
            }
        }
        if (result.entity && Memory::isAllocated((uintptr_t)result.entity))
        {
            ImGui::Separator();
            ImGui::Text("Entity: %p", (void *)result.entity);
            ImGui::Text("Entity Tag ID: %X", result.entity->tagID);
            if (ImGui::CollapsingHeader("Entity Data..."))
            {
                ImGui::Indent(20.0f);
                interpretPointer(result.entity);
                ImGui::Unindent(20.0f);
            }
        }
    }

    void interpretPointer(void *entity) {
        // Copyable pointer text:
        ImGui::Text("Object at: %p", entity);
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy pointer to clipboard");
        if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
        {
            char text[255] = {0};
            snprintf(text, 255, "%p", entity);
            ImGui::SetClipboardText(text);
        }

        // Scrollable region with all the entity's data interpreted as u32 values.
        ImGui::BeginChild("Object Data", ImVec2(0, 0), true, ImGuiChildFlags_Border | ImGuiChildFlags_AutoResizeY);
        if (!entity)
        {
            ImGui::Text("Object is null.");
            ImGui::EndChild();
            return;
        }
        if (!Memory::isAllocated((uintptr_t)entity))
        {
            ImGui::Text("Nothing allocated at this address.");
            ImGui::EndChild();
            return;
        }
        uint32_t *entityAddr = (uint32_t *)entity;
        for (int i = 0; i < 200; i++)
        {
            if (!Memory::isAllocated((uintptr_t)&entityAddr[i]))
                break;
            uint32_t value = entityAddr[i];
            if (value == 0)
                continue;
            if (!Halo1::hasInterpretation(value))
                continue;
            char label[255] = {0};

            // As fourcc;
            auto groupId = Strings::fourccToString(value);

            snprintf(label, 255, "%02X: %08X - %s", i * 4, value, groupId.c_str());
            if (ImGui::CollapsingHeader(label))
            {
                ImGui::Indent(20.0f);
                interpretations(value);
                ImGui::Unindent(20.0f);
            }
        }
        ImGui::EndChild();
    }   
}
