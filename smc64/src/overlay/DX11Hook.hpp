#pragma once
#include <Windows.h>

namespace DX11Hook {
    HWND findMainWindow();
    
    void hook(HWND hwnd);
    void unhook();
}