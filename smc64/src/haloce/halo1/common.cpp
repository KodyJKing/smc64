#include "common.hpp"

#include <Windows.h>

namespace Halo1 {
    uintptr_t dllBase() {
        return (uintptr_t) GetModuleHandleA( "halo1.dll" );
    }
}
