#pragma once

#include <string>

namespace Strings {
    std::string fourccToString(uint32_t fourcc);
    uint32_t stringToFourcc(const std::string & str);
}