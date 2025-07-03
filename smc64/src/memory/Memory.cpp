#include "Memory.hpp"
#include <vector>

namespace Memory {

    bool isAllocated( uintptr_t address ) {
        MEMORY_BASIC_INFORMATION mbi;
        if ( !VirtualQuery( (LPCVOID) address, &mbi, sizeof( mbi ) ) )
            return false;
        return mbi.State == MEM_COMMIT;

        // uintptr_t dummy;
        // return ReadProcessMemory(GetCurrentProcess(), (LPCVOID)address, &dummy, sizeof(dummy), NULL);
    }

}