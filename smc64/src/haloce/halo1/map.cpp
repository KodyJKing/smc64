#include "map.hpp"
#include "common.hpp"
#include "memory/Memory.hpp"

namespace Halo1 {

    static const uintptr_t relocatedMapBaseOffset = 0x2D9CE10U;
    static const uintptr_t mapBaseOffset = 0x2EA3410U;

    uint64_t translateMapAddress( uint32_t address ) {
        uint64_t relocatedMapBase = *(uint64_t*) ( dllBase() + relocatedMapBaseOffset );
        uint64_t mapBase = *(uint64_t*) ( dllBase() + mapBaseOffset );
        return address + ( relocatedMapBase - mapBase );
    }
    
    uint32_t translateToMapAddress( uint64_t absoluteAddress ) {
        uint64_t relocatedMapBase = *(uint64_t*) ( dllBase() + relocatedMapBaseOffset );
        uint64_t mapBase = *(uint64_t*) ( dllBase() + mapBaseOffset );
        return (uint32_t) ( absoluteAddress - ( relocatedMapBase - mapBase ) );
    }

        char* getMapName() {
        MapHeader* header = getMapHeader();
        if ( !header ) return nullptr;
        return header->mapName;
    }

    bool checkMapHeader(MapHeader* header) {
        if (!header) {
            // std::cout << "Error: header is null" << std::endl;
            return false;
        }
        if ( !Memory::isAllocated( (uintptr_t) header ) ) {
            // std::cout << "Error: header is not allocated" << std::endl;
            return false;
        }
        if (header->magicHeader != 1751474532) {
            // std::cout << "Error: magicHeader is not 1751474532" << std::endl;
            return false;
        }
        if (header->magicFooter != 1718579060) {
            // std::cout << "Error: magicFooter is not 1718579060" << std::endl;
            // std::cout << offsetof(MapHeader, magicFooter) << std::endl;
            return false;
        }
        return true;
    }

    MapHeader* getMapHeader() {
        MapHeader* result = (MapHeader*) ( dllBase() + 0x2B22744U );
        if ( !checkMapHeader( result ) )
            return nullptr;
        return result;
    }

    bool isOnMap( const char* mapName ) {
        auto actualMapName = getMapName();
        if ( !actualMapName )
            return false;
        return strncmp( mapName, actualMapName, strnlen( mapName, 32 ) ) == 0;
    }

    bool isMapLoaded() {
        auto header = getMapHeader();
        return header && Memory::isAllocated( (uintptr_t) header ) && checkMapHeader( header );
    }

}