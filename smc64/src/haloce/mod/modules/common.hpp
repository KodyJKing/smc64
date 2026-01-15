#pragma once

#include <iostream>
#include <cstdint>
#include <Windows.h>
#include "MinHook.h"
#include "asm/AsmHelper.hpp"
#include "utils/UnloadLock.hpp"
#include "memory/Memory.hpp"
#include "../halo1/halo1.hpp"

#define HOOK_FUNC( func, offset) \
    void* p##func = (void*) (halo1 + offset); \
    std::cout << #func << ": " << std::endl; \
    std::cout << AsmHelper::disassemble( (uint8_t*) p##func, 0x100 ) << std::endl; \
    MH_CreateHook( p##func, hk##func, (void**) &original##func ); \
    MH_EnableHook( p##func );
