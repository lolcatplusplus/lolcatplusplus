#pragma once

#include <iostream>
#include <memory>
#include <string>

/**
 * Open a file and return a stream which yeilds UTF-8 data.
 * This will convert UTF-16 (LE/BE)
 *
 * This might throw a runtime exception.
 */
std::unique_ptr<std::istream> open_file(const std::string &path);
