#pragma once

#include <stdint.h>

#define NULL_HANDLE 0xFFFFFFFF

namespace Halo1 {
    uintptr_t dllBase();
    uint64_t translateMapAddress( uint32_t address );
    uint32_t translateToMapAddress(uint64_t absoluteAddress);
}
