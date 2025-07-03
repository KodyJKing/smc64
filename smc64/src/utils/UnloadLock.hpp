// Provides an RAII type that prevents mod from unloading while certain work is being done.
#pragma once

#include <stdint.h>

void waitForSafeUnload(uint32_t pollRate = 100);

class UnloadLock {
    public:
        UnloadLock();
        ~UnloadLock();
};