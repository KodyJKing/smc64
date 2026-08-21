#include "functions/DumpMarioGeometry.hpp"
#include "debug/MarioSoundBoard.hpp"
#include "imgui.h"

namespace Mod::Mario {

    void renderPauseMenuTab() {
        
        #ifdef _DEBUG
        if (ImGui::BeginTabItem("Mario")) {
            if (ImGui::Button("Dump Mario Geometry")) {
                dumpMarioGeometry();
            }
            renderSoundBoardButton();
            ImGui::EndTabItem();
        }
        renderSoundBoard();
        #endif
        
    }

}
