// Reverse engineered Halo 1 structures and access functions go here.

#pragma once

#include <stdint.h>
#include <string>

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

    Tag* getTag( uint32_t tagID );
    Tag * findTag(const char * path, const char * fourCC);
    Tag * findTag(const char * path, uint32_t fourCC);
    bool validTagPath(const char * path);
    bool tagExists(Tag * tag);
}