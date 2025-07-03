// Provides an RAII type that prevents mod from unloading while certain work is being done.

#include "UnloadLock.hpp"
#include <atomic>
#include <Windows.h>

std::atomic<uint64_t> lockCount;

void waitForSafeUnload(uint32_t pollRate) {
    while (lockCount > 0) {
        Beep(750, 100);
        Sleep(pollRate);
    }
}

UnloadLock::UnloadLock() {
    lockCount++;
}

UnloadLock::~UnloadLock() {
    lockCount--;
}
