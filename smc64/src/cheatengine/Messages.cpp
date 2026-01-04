#include "Messages.hpp"
#include "IPC.hpp"
#include <string>

enum class MessageType : uint64_t {
    OpenHexView = 1,
};

namespace CE::Messages {
    void openHexView(uintptr_t address, std::string caption) {
        struct Message {
            MessageType type;
            uintptr_t address;
            char caption[128]; // Optional caption for the hex view
        } message = {
            MessageType::OpenHexView,
            address
        };
        strncpy_s(message.caption, caption.c_str(), sizeof(message.caption) - 1);
        CE::IPC::sendMessage(&message, sizeof(message));
    }
}
