#include "Zydis.h"
#include <string>
#include <sstream>
#include <iomanip>
#include <format>

namespace AsmHelper {

    std::string disassemble(uint8_t* data, size_t size) {
        std::stringstream result;

        ZyanUSize offset = 0;
        ZydisDisassembledInstruction instruction;
        ZyanU64 runtimeAddress = (ZyanU64) data;
        while ( ZYAN_SUCCESS( ZydisDisassembleIntel(
            /* machine_mode:    */ ZYDIS_MACHINE_MODE_LONG_64,
            /* runtime_address: */ runtimeAddress,
            /* buffer:          */ data + offset,
            /* length:          */ size - offset,
            /* instruction:     */ &instruction
        ) ) ) {
            auto instructionLength = instruction.info.length;

            #ifdef _WIN64
                result << std::setfill( '0' ) << std::setw( 16 ) << std::right << (void*)runtimeAddress;
            #else
                result << std::setfill( '0' ) << std::setw( 8 ) << std::right << (void*)runtimeAddress;
            #endif
            result << "  ";
            result << std::setfill( ' ' ) << std::setw( 50 ) << std::left << instruction.text;
            result << " ";

            for ( int i = 0; i < instructionLength; i++ ) {
                result << std::format( "{:02X}", data[offset + i] );
                if ( i < instructionLength - 1 ) result << " ";
            }

            // if ( instruction.info.attributes & ZYDIS_ATTRIB_IS_RELATIVE )
            //     result << " (relative)";

            result << std::endl;

            offset += instructionLength;
            runtimeAddress += instructionLength;
        }
        
        return result.str();
    }

}