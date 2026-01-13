// Reverse engineered Halo 1 structures and access functions go here.

#pragma once

#include <stdint.h>
#include <string>
#include "map.hpp"
#include "memory/Memory.hpp"

namespace Halo1 {
    // Thanks to Kavawuvi for documentation on the map format and tag structure.
    class Tag {
        public:
        uint32_t groupID; // Group ID's are fourcc's
        uint32_t parentGroupID; 
        uint32_t grandparentGroupID;
        uint32_t tagID; 
        uint32_t resourcePathAddress; 
        uint32_t dataAddress; 
        char pad_0018[8]; 

        char* getResourcePath();
        void* getData();
        std::string groupIDStr();
    };

    // Represents a block of structs in a tag's data.
    struct BlockPointer {
        uint32_t count;
        uint32_t offset;   // Use translateMapAddress to get the actual pointer
        uint32_t bullshit; // Not sure what this does.

        template <typename T>
        T* get(size_t index, bool safe = true) {
            if (index >= count) return nullptr;
            uint64_t baseAddress = Halo1::translateMapAddress(offset);
            if (!baseAddress) return nullptr;
            if (safe && !Memory::isAllocated(baseAddress + index * sizeof(T))) return nullptr;
            return (T*)(baseAddress + index * sizeof(T));
        }
    };

    Tag* getTag( uint32_t tagID );
    Tag * findTag(const char * path, const char * fourCC);
    Tag * findTag(const char * path, uint32_t fourCC);
    bool validTagPath(const char * path);
    bool tagExists(Tag * tag);
}