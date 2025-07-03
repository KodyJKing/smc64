#pragma once
#include <Windows.h>

namespace Overlay {
        HWND getGameWindow();
        void initializeContext(HWND targetWindow);
        void render();
        void init();
        void free();
}