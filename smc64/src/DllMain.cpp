#include <Windows.h>
#include "DllMain.hpp"
#include <iostream>
#include <filesystem>
#include "utils/Console.hpp"
#include "utils/UnloadLock.hpp"
#include "utils/Utils.hpp"
#include "haloce/Mod.hpp"
#include "MinHook.h"
#include "overlay/Overlay.hpp"

namespace ModHost {
    bool bReinit = false;
    bool bExit = false;
    void reinitialize() {
        std::cout << "Reinitializing..." << std::endl;
        bReinit = true;
    }
    void exit() {
        std::cout << "Exiting..." << std::endl;
        bExit = true;
    }
}


// MainThread
DWORD WINAPI MainThread(LPVOID _hModule) {
    HMODULE hModule = (HMODULE) _hModule;
    Console::alloc();
    Console::toggleConsole();

    std::cout << "Load method: " << (Utils::isInjected() ? "Injection" : "Imported") << std::endl;
    if (!Utils::isInjected()) { // Give the game some time to load.
        // This module seems to load late in the game's initialization.
        Utils::waitForModule("PartyWin.dll");
    }

    do {
        ModHost::bReinit = false;
        
        MH_Initialize();
        Overlay::init();
        
        while (!ModHost::bExit && !ModHost::bReinit) {
            if (GetAsyncKeyState(VK_F8) & 1)
                Console::toggleConsole();
            if ( Utils::isInjected() && (GetAsyncKeyState(VK_F9) & 1) ) { // Only allow uninjecting if the mod was injected.
                ModHost::exit();
                break;
            }
            if (GetAsyncKeyState(VK_F10) & 1) {
                ModHost::reinitialize();
                break;
            }
            HaloCE::Mod::modThreadUpdate();
            Sleep(1000 / 60);
        }
        
        waitForSafeUnload();
        HaloCE::Mod::free();
        Overlay::free();
        MH_Uninitialize();
        
    } while (ModHost::bReinit);


    waitForSafeUnload();
    Sleep(200); // Extra time for hooks to exit.

    Console::free();

    FreeLibraryAndExitThread(hModule, 0);
    return 0;
}

// Dll Main
// We dll export this so MSDetours setdll.exe can add it to a carrier dll.
BOOL __declspec(dllexport) APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    switch (ul_reason_for_call) {
        case DLL_PROCESS_ATTACH:
            CreateThread(0, 0, MainThread, hModule, 0, 0);
            break;
        case DLL_THREAD_ATTACH:
            break;
        case DLL_THREAD_DETACH:
            break;
        case DLL_PROCESS_DETACH:
            break;
    }
    return TRUE;
}