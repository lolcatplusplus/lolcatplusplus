#pragma once

#include <cstddef>
#include <string_view>

namespace utf8 {

/**
 * Get the length (1-4) of a UTF-8 sequence based on the first byte
 */
[[nodiscard]] size_t get_sequence_length(unsigned char first_byte);

/**
 * Check if the (sub)string is valid UTF-8
 */
[[nodiscard]] bool is_valid(std::string_view text);

} // namespace utf8
