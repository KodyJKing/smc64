#include <Windows.h>
#include <stdint.h>
#include <thread>
#include <mutex>
#include <string>
#include "imgui.h"
#include "Halo1.hpp"
#include "utils/Strings.hpp"

namespace HaloCE::Mod::UI {

    struct GroupID {
        const char* name;
        uint32_t groupID;
    };

    #define GROUP_ID_ALL 0

    GroupID ids[] = {
        {"All", GROUP_ID_ALL},
        #define GROUP_ID(friendlyName, fourccStr) {friendlyName, Strings::stringToFourcc(fourccStr)},
        GROUP_ID("Weapon", "weap")
        GROUP_ID("Projectile", "proj")
        GROUP_ID("Damage", "jpt!")
        GROUP_ID("Vehicle", "vehi")
        GROUP_ID("Biped", "bipd")
        GROUP_ID("Scenenery", "scen")
        GROUP_ID("Device", "devi")
        GROUP_ID("Effect", "effe")
        GROUP_ID("Contrail", "cont")
        GROUP_ID("Particle", "part")
        GROUP_ID("Sound", "snd!")
        GROUP_ID("Animation", "antr")
        GROUP_ID("Actor", "actr")
        GROUP_ID("Actor Variant", "actv")
        GROUP_ID("Bitmap", "bitm")
        GROUP_ID("Shader", "shdr")
        GROUP_ID("Light", "ligh")
        GROUP_ID("BSP", "sbsp")
        #undef GROUP_ID
    };

    #define NUM_GROUP_IDS (sizeof(ids) / sizeof(GroupID))

    bool showTagBrowser = false;

    void tagBrowser() {

        ImGui::Begin("Tag Browser", &showTagBrowser, ImGuiWindowFlags_AlwaysAutoResize);
        
            //////////////////////////////////////////////////////////////////////////
            // Pagination

            static int tagsPerPage = 50;
            static int page = 0;
            ImGui::InputInt("Page", &page);
            if (ImGui::IsWindowHovered())
                page -= (int) ImGui::GetIO().MouseWheel;
            if (page < 0) 
                page = 0;
            ImGui::SameLine();
            ImGui::InputInt("Page size", &tagsPerPage);
            if (tagsPerPage < 1) 
                tagsPerPage = 1;

            //////////////////////////////////////////////////////////////////////////
            // Search

            static int groupIdFilterIndex = 0;
            static int lastGroupIdFilterIndex = 0;
            static char search[512] = {0};
            static char lastSearch[512] = {0};
            static std::vector<int> searchResults;

            ImGui::Separator();
            { // GroupID filter input
                if (ImGui::BeginCombo("##GroupID", ids[groupIdFilterIndex].name)) {
                    for (int i = 0; i < NUM_GROUP_IDS; i++) {
                        bool isSelected = groupIdFilterIndex == i;
                        if (ImGui::Selectable(ids[i].name, isSelected)) {
                            groupIdFilterIndex = i;
                        }
                        if (isSelected) ImGui::SetItemDefaultFocus();
                    }
                    ImGui::EndCombo();
                }
            }

            ImGui::SameLine();
            ImGui::InputText("Search", search, 512);
            bool focused = ImGui::IsItemActive();
            
            ImGui::SameLine();
            if (ImGui::Button("Clear")) {
                search[0] = 0;
                groupIdFilterIndex = 0;
            }

            bool hasSearch = search[0] != 0 || groupIdFilterIndex != 0;

            auto filterTag = [&](Halo1::Tag* tag) {
                auto filterGroupID = ids[groupIdFilterIndex].groupID;
                if (
                    filterGroupID != GROUP_ID_ALL && 
                    filterGroupID != tag->groupID &&
                    filterGroupID != tag->parentGroupID &&
                    filterGroupID != tag->grandparentGroupID
                ) 
                    return false;
                if (search[0] == 0)
                    return true;
                std::string pathStr = tag->getResourcePath();
                return pathStr.find(search) != std::string::npos;
            };
            
            bool searchChanged = strcmp(search, lastSearch) != 0 || groupIdFilterIndex != lastGroupIdFilterIndex;

            // Debounce search
            static uint64_t lastSearchTick = GetTickCount64();
            uint64_t tick = GetTickCount64();
            bool debounce = focused && (tick - lastSearchTick < 200);
            
            static std::mutex searchMutex;
            if (!debounce && searchChanged && searchMutex.try_lock() ) {
                searchMutex.unlock();
                
                std::thread searchThread = std::thread( [&] {
                    std::lock_guard<std::mutex> lock(searchMutex);
                    std::string searchStr = search;

                    bool canceledSearch = false;

                    bool hasLastSearch = lastSearch[0] != 0;
                    bool containsLastSearch = hasLastSearch && searchStr.find(lastSearch) != std::string::npos;
                    bool filterNarrowed = lastGroupIdFilterIndex == groupIdFilterIndex || ids[lastGroupIdFilterIndex].groupID == GROUP_ID_ALL;
                    bool canReuse = containsLastSearch && filterNarrowed;
                    if (canReuse) {
                        // Results will be a subset of previous results, filter them.
                        std::vector<int> newResults;
                        for (int index : searchResults) {
                            auto tag = Halo1::getTag(index);
                            if (filterTag(tag))
                                newResults.push_back(index);
                        }
                        searchResults = newResults;
                    } else {
                        // Search from scratch
                        searchResults.clear();
                        int i = 0;
                        while (!canceledSearch) {
                            auto tag = Halo1::getTag(i);
                            if (!Halo1::tagExists(tag)) 
                                break;
                            if (filterTag(tag))
                                searchResults.push_back(i);
                            i++;

                            // Cancel search if query has changed
                            if (searchStr != search)
                                canceledSearch = true;
                        }
                    }
                    if (!canceledSearch) {
                        strcpy_s( lastSearch, searchStr.c_str() );
                        lastGroupIdFilterIndex = groupIdFilterIndex;
                    }
                    lastSearchTick = GetTickCount64();
                } );
                searchThread.detach();

            }

            if (hasSearch)  {
                int numPages = (int) ceil( searchResults.size() / (float) tagsPerPage );
                ImGui::Text("%d results, %d pages", searchResults.size(), numPages);
                if (page >= numPages) page = numPages - 1;
                if (page < 0) page = 0;
            }

            //////////////////////////////////////////////////////////////////////////
            // Render tag item

            auto renderTag = [&](int index) {
                if (index < 0) {
                    ImGui::Text("");
                    return;
                }

                auto tag = Halo1::getTag(index);
                if (Halo1::tagExists(tag)) {
                    auto path = tag->getResourcePath();
                    if (!Halo1::validTagPath(path)) {
                        ImGui::Text("%04d NULL", index);
                    } else {
                        char text[2048] = {0};

                        ImGui::Text("%04d", index);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy index to clipboard");
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( std::to_string(index).c_str() );

                        ImGui::SameLine();
                        sprintf_s( text, "%X", tag->tagID );
                        ImGui::Text(text);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy TagID to clipboard");
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( text );

                        // ImGui::SameLine();
                        // sprintf_s( text, "%p", tag );
                        // ImGui::Text( "%s", text );
                        // if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy tag pointer to clipboard");
                        // if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( text );

                        ImGui::SameLine();
                        auto groupID = tag->groupIDStr();
                        ImGui::Text("%s", groupID.c_str());
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy GroupID to clipboard");
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( groupID.c_str() );

                        ImGui::SameLine();
                        auto pDataAddress = &tag->dataAddress;
                        sprintf_s( text, "%p", pDataAddress );
                        ImGui::Text("%s", text);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy data address pointer to clipboard");
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( text );

                        ImGui::SameLine();
                        auto data = tag->getData();
                        sprintf_s( text, "%p", data );
                        ImGui::Text("%s", text);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy data pointer to clipboard");
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( text );

                        ImGui::SameLine();
                        ImGui::Text("%s", path);
                        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Right click to copy path to clipboard");
                        if (ImGui::IsItemClicked(ImGuiMouseButton_Right)) ImGui::SetClipboardText( path );

                    }
                } else {
                    ImGui::Text("%04d NULL", index);
                }
            };

            //////////////////////////////////////////////////////////////////////////
            // Results

            ImGui::Separator();

            if (hasSearch) {
                // Render search results
                if (searchMutex.try_lock()) {
                    int pageBase = page * tagsPerPage;
                    for (int i = 0; i < tagsPerPage; i++) {
                        int j = pageBase + i;
                        int index = -1;
                        if (j < searchResults.size())
                            index = searchResults[j];
                        renderTag(index);
                    }
                    searchMutex.unlock();
                } else {
                    ImGui::Text("Searching...");
                }
            } else {
                // Render all tags
                for (int i = 0; i < tagsPerPage; i++) {
                    int index = page * tagsPerPage + i;
                    auto tag = Halo1::getTag(index);
                    renderTag(index);
                }
            }

        ImGui::End();

    }

}