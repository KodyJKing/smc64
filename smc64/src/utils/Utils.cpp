#include <Windows.h>
#include <string>
#include <filesystem>
#include "Utils.hpp"

EXTERN_C IMAGE_DOS_HEADER __ImageBase;

namespace Utils {

    HMODULE waitForModule(std::string moduleName, DWORD sleepTime, DWORD timeout) {
        HMODULE module = nullptr;
        DWORD startTick = GetTickCount();
        while (!module) {
            module = GetModuleHandleA(moduleName.c_str());
            Sleep(sleepTime);
            if (timeout && GetTickCount() - startTick > timeout)
                return nullptr;
        }
        return module;
    }

    static HMODULE getCurrentImageBase( ) {
		return (HINSTANCE)(&__ImageBase);
	}

    /*
        Mod can either be injected or added to the game's import table. 
        If MCC-Win64-Shipping.exe is in the same directory, we are not injected.
    */
    bool isInjected() {
        static bool cached = false;
        static bool result = false;

        if (cached)
            return result;

        HMODULE hModule = getCurrentImageBase();
        char path[MAX_PATH];
        GetModuleFileNameA(hModule, path, MAX_PATH);
        std::filesystem::path modulePath = path;
        std::filesystem::path exePath = modulePath.parent_path() / "MCC-Win64-Shipping.exe";

        result = !std::filesystem::exists(exePath);
        cached = true;
        return result;
    }

}