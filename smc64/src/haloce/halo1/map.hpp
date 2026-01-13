#pragma once

#include <stdint.h>

namespace Halo1 {

    #pragma pack(push, 1)
    struct MapHeader {
        uint32_t magicHeader; // Should be 1751474532 ('head' in ascii fourcc)
        uint32_t cacheVersion;
        uint32_t fileSize;
        uint32_t paddingLength; // Only used on Xbox
        uint32_t tagDataOffset;
        uint32_t tagDataSize;
        char pad0[8];
        char mapName[32];
        char buildVersion[32];
        uint32_t scenarioType;
        uint32_t checksum;
        char pad1[0x794];
        uint32_t magicFooter; // Should be 1718579060 ('foot' in ascii fourcc)
    };
    #pragma pack(pop)
    
    char* getMapName();
    MapHeader* getMapHeader();
    bool isOnMap( const char* mapName );
    bool isMapLoaded();
    uint64_t translateMapAddress( uint32_t address );
    uint32_t translateToMapAddress(uint64_t absoluteAddress);

}