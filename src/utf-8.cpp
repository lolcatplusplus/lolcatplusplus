#include "utf-8.hpp"

namespace utf8 {

size_t get_sequence_length(const unsigned char c) {
    if ((c & 0x80) == 0) {
        return 1;
    } else if ((c & 0xE0) == 0xC0) {
        return 2;
    } else if ((c & 0xF0) == 0xE0) {
        return 3;
    } else if ((c & 0xF8) == 0xF0) {
        return 4;
    }
    return 1;
}

bool is_valid(std::string_view text) {
    size_t i = 0;
    while (i < text.size()) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        if ((c & 0x80) == 0) {
            i++;
            continue;
        }

        size_t len = 0;
        if ((c & 0xE0) == 0xC0)
            len = 2;
        else if ((c & 0xF0) == 0xE0)
            len = 3;
        else if ((c & 0xF8) == 0xF0)
            len = 4;
        else
            return false; // Invalid start byte

        if (i + len > text.size())
            return true;

        // continuation bytes
        for (size_t j = 1; j < len; ++j) {
            if ((static_cast<unsigned char>(text[i + j]) & 0xC0) != 0x80)
                return false;
        }

        if (len == 2 && c < 0xC2)
            return false;

        i += len;
    }
    return true;
}

} // namespace utf8
