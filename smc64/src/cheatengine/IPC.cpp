#include "IPC.hpp"
#include "Windows.h"
#include <string>

#define DLL_EXPORT __declspec(dllexport)

#define IPC_BUFFER_SIZE 128
#define IPC_MESSAGE_SIZE 1024

uint64_t calculateChecksum(const void* data, size_t dataSize) {
    const uint8_t* bytes = static_cast<const uint8_t*>(data);
    uint64_t checksum = 0;
    for (size_t i = 0; i < dataSize; ++i) {
        checksum += bytes[i];
    }
    return checksum;
}

struct Message {
    uint64_t seq;
    uint64_t checksum;
    uint32_t length;
    char data[IPC_MESSAGE_SIZE];
};

struct MessageBuffer {
    Message messages[IPC_BUFFER_SIZE];
    uint64_t writeIndex;
};

MessageBuffer _messageBuffer = {};

extern "C" DLL_EXPORT MessageBuffer* messageBuffer = &_messageBuffer;
extern "C" DLL_EXPORT size_t messageBufferSize = sizeof(MessageBuffer);
extern "C" DLL_EXPORT size_t messageSize = sizeof(Message);

namespace CE::IPC {
    void sendMessage(const void* data, size_t dataSize) {
    if (dataSize == 0 || data == nullptr) {
        return; // No data to write
    }

    if (dataSize > IPC_MESSAGE_SIZE) {
        return;
    }

    MessageBuffer* messageBuffer = &::_messageBuffer;

    uint64_t writeIndex = messageBuffer->writeIndex % IPC_BUFFER_SIZE;
    Message& msg = messageBuffer->messages[writeIndex];

    msg.seq = messageBuffer->writeIndex;
    msg.length = static_cast<uint32_t>(dataSize > sizeof(msg.data) ? sizeof(msg.data) : dataSize);
    memcpy(msg.data, data, msg.length);
    msg.checksum = calculateChecksum(msg.data, msg.length);

    // Update write index
    messageBuffer->writeIndex++;
    }
}
