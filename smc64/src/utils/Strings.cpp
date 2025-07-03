#include "Strings.hpp"
#include <stdint.h>

namespace Strings {

    std::string fourccToString(uint32_t fourcc) {
        char str[5];
        str[0] = (fourcc >> 24) & 0xFF;
        str[1] = (fourcc >> 16) & 0xFF;
        str[2] = (fourcc >> 8) & 0xFF;
        str[3] = fourcc & 0xFF;

        for (int i = 0; i < 4; i++) {
            char c = str[i];
            if (
                (c < '0' || c > '9') &&
                (c < 'A' || c > 'Z') &&
                (c < 'a' || c > 'z') &&
                c != '!'
            ) str[i] = ' ';
        }

        str[4] = 0;
        return std::string(str);
    }

    uint32_t stringToFourcc(const std::string& str) {
        char a, b, c, d;
        a = str[0];
        b = str[1];
        c = str[2];
        d = str[3];
        return (a << 24) | (b << 16) | (c << 8) | d;
    }

}