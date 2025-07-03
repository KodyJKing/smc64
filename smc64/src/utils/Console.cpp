#include <Windows.h>
#include <iostream>
#include "Console.hpp"

namespace Console {
    FILE *f;

    void alloc() {
        AllocConsole();
        freopen_s(&f, "CONOUT$", "w", stdout);
        SetConsoleTitleA("SuperHot MCC");

        // Hide close button on console window (closing the console seems to close/crash the game)
        HWND consoleWindow = GetConsoleWindow();
        HMENU hMenu = GetSystemMenu(consoleWindow, FALSE);
        EnableMenuItem(hMenu, SC_CLOSE, MF_BYCOMMAND | MF_GRAYED);
    }

    void free() {
        fclose(f);
        FreeConsole();
    }

    bool isConsoleVisible() {
        HWND console = GetConsoleWindow();
        return IsWindowVisible(console) && !IsIconic(console);
    }

    void toggleConsole() {
        HWND console = GetConsoleWindow();
        if (isConsoleVisible()) {
            ShowWindow(console, SW_HIDE);
        } else {
            if (IsIconic(console))
                ShowWindow(console, SW_RESTORE);
            else
                ShowWindow(console, SW_SHOW);
            SetWindowPos(console, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
        }
    }
}