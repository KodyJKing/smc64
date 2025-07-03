#pragma once

#include <Windows.h>
#include <string>

namespace Utils {
    HMODULE waitForModule(std::string moduleName, DWORD sleepTime = 100, DWORD timeout = 0);
    bool isInjected();
}
