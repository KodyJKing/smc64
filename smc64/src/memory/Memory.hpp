#pragma once
#include <Windows.h>
#include <optional>
#include <vector>
#include <memory>

namespace Memory {

    bool isAllocated(uintptr_t address);

    template<typename T>
    std::optional<T> safeRead(uintptr_t address) {
        T buffer;
        if(address && ReadProcessMemory(GetCurrentProcess(), (LPCVOID)address, &buffer, sizeof(T), NULL))
            return buffer;
        return std::nullopt;
    }

    //  Safe for unallocated memory, but does not check page protection.
    template<typename T>
    bool safeWriteFast(uintptr_t address, T value) {
        if (isAllocated(address)) {
            T* ptr = (T*)address;
            *ptr = value;
            return true;
        }
        return false;
    }

    template<typename T>
    bool safeWrite(uintptr_t address, T value) {
        DWORD oldProtect;
        if (!VirtualProtect((LPVOID)address, sizeof(T), PAGE_EXECUTE_READWRITE, &oldProtect))
            return false;
        bool result = safeWriteFast(address, value);
        VirtualProtect((LPVOID)address, sizeof(T), oldProtect, &oldProtect);
        return result;
    }

    class Patch {
        public:
            Patch(void* address, std::vector<uint8_t>& patchBytes) : m_address(address) {
                m_originalBytes.resize(patchBytes.size());
                ReadProcessMemory(GetCurrentProcess(), address, m_originalBytes.data(), patchBytes.size(), NULL);
                WriteProcessMemory(GetCurrentProcess(), address, patchBytes.data(), patchBytes.size(), NULL);
                m_patchBytes = patchBytes;
            }
            ~Patch() {
                if (!isAllocated((uintptr_t)m_address))
                    return;
                // Only restore if the patch is still applied.
                std::vector<uint8_t> currentBytes(m_patchBytes.size());
                ReadProcessMemory(GetCurrentProcess(), m_address, currentBytes.data(), m_patchBytes.size(), NULL);
                if (currentBytes == m_patchBytes)
                    WriteProcessMemory(GetCurrentProcess(), m_address, m_originalBytes.data(), m_originalBytes.size(), NULL);
            }
        private:
            std::vector<uint8_t> m_originalBytes;
            std::vector<uint8_t> m_patchBytes;
            void* m_address;
    };

    using PatchPtr = std::shared_ptr<Patch>;

    static PatchPtr createPatch(void* address, std::vector<uint8_t> patchBytes) {
        return std::make_shared<Patch>(address, patchBytes);
    }

    template<typename T>
    PatchPtr createPatch(T& lValue, T value) {
        std::vector<uint8_t> patchBytes(sizeof(T));
        memcpy(patchBytes.data(), &value, sizeof(T));
        return createPatch(&lValue, patchBytes);
    }
    
}
