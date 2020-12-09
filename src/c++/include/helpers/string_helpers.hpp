#ifndef STRING_HELPERS_HPP
#define STRING_HELPERS_HPP

namespace Helpers {

// Standard Includes
#include <string>
#include <vector>
#include <algorithm>

/**
 * @brief Splits a string into multiple elements within a vector
 * @param delim What to split by (not included in produced vectors' elements)
 * @param to_split The string to split
 * @return The split string in the form of a vector
 */
inline const std::vector<std::string> splitStr(const char delim, const std::string to_split) {
    std::vector<std::string> split_str;

    // iterate over string (backwards bc of push_back)
    std::string::size_type beg = 0;
    for (std::size_t end = 0; (end = to_split.find(delim, end)) != std::string::npos; ++end) {
        split_str.push_back(to_split.substr(beg, end - beg));
        beg = end + 1;
    }
    split_str.push_back(to_split.substr(beg));

    return std::move(split_str);
}


}; // end of Helpers namespace
#endif