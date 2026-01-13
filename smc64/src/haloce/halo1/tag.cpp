#include "tag.hpp"
#include "common.hpp"
#include "utils/Strings.hpp"
#include "memory/Memory.hpp"

namespace Halo1 {
    
    char* Tag::getResourcePath() { return (char*) translateMapAddress( resourcePathAddress ); }
    void* Tag::getData() { return (void*) translateMapAddress( dataAddress ); }
    std::string Tag::groupIDStr() {
        auto fourccA = Strings::fourccToString( groupID );
        auto fourccB = Strings::fourccToString( parentGroupID );
        auto fourccC = Strings::fourccToString( grandparentGroupID );
        return "[" + fourccC + " > " + fourccB + " > " + fourccA + "]";
    }

    Tag* getTag( uint32_t tagID ) {
        if (tagID == NULL_HANDLE)
            return nullptr;
        Tag* tagArray = *(Tag**) ( dllBase() + 0x1C34FB0U );
        return &tagArray[tagID & 0xFFFF];
    }

    Tag* findTag( const char* path, uint32_t fourCC) {
        if ( !validTagPath( path ) ) return nullptr;
        Tag* tagArray = *(Tag**) ( dllBase() + 0x1C34FB0U );
        for ( uint32_t i = 0; i < 0x10000; i++ ) {
            auto tag = &tagArray[i];
            if ( !tagExists( tag ) ) break;
            if ( strcmp( tag->getResourcePath(), path ) == 0 && tag->groupID == fourCC )
                return tag;
        }
        return nullptr;
    }

    Tag* findTag( const char* path, const char* fourCC ) {
        uint32_t fourCCValue = Strings::stringToFourcc( fourCC );
        return findTag( path, fourCCValue );
    }

    bool tagExists( Tag* tag ) {
        return tag && Memory::isAllocated( (uintptr_t) tag->getData() ) && Memory::isAllocated( (uintptr_t) tag->getResourcePath() );
    }

        bool validTagPath( const char* path ) {
        // Must match [a-zA-Z0-9_ \.\\-]+
        // Must be atleast 3 characters long.
        // Also must contain atleast one backslash.
        int backslashCount = 0;
        for ( size_t i = 0; i < 512; i++ ) {
            char c = path[i];
            if ( c == 0 ) 
                return backslashCount > 0 && i > 2;
            if ( 
                !(c >= 'a' && c <= 'z') &&
                !(c >= 'A' && c <= 'Z') &&
                !(c >= '0' && c <= '9') && 
                c != '_'   && c != ' '  &&
                c != '\\'  && c != '.'  && c != '-'
            )
                return false;
            if ( c == '\\' ) 
                backslashCount++;
        }
        return true;
    }
    
}
