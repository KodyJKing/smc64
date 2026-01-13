#include "common.hpp"

#include <Windows.h>

namespace Halo1 {

    uintptr_t dllBase() {
        return (uintptr_t) GetModuleHandleA( "halo1.dll" );
    }

    static const uintptr_t relocatedMapBaseOffset = 0x2D9CE10U;
    static const uintptr_t mapBaseOffset = 0x2EA3410U;

    uint64_t translateMapAddress( uint32_t address ) {
        uint64_t relocatedMapBase = *(uint64_t*) ( dllBase() + relocatedMapBaseOffset );
        uint64_t mapBase = *(uint64_t*) ( dllBase() + mapBaseOffset );
        return address + ( relocatedMapBase - mapBase );
    }
    
    uint32_t translateToMapAddress( uint64_t absoluteAddress ) {
        uint64_t relocatedMapBase = *(uint64_t*) ( dllBase() + relocatedMapBaseOffset );
        uint64_t mapBase = *(uint64_t*) ( dllBase() + mapBaseOffset );
        return (uint32_t) ( absoluteAddress - ( relocatedMapBase - mapBase ) );
    }
}
