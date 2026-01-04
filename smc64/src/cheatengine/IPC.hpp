#pragma once
#include <cstdint>

namespace CE::IPC {
    void sendMessage(const void* data, size_t dataSize);
}
