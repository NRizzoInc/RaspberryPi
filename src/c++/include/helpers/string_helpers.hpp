#ifndef STRING_HELPERS_HPP
#define STRING_HELPERS_HPP

// Standard Includes
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

namespace Helpers {

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

/**
 * @brief Creates a string containing all elements of the vector (comma-seperated)
 * @param vec The vector to print
 * @param sep What to print in between elements (defaults to ", ") 
 * @return std::string 
 */
template <typename vecType>
inline std::string createVecStr(const std::vector<vecType>& vec, const std::string sep=", ") {
    std::stringstream to_return;
    const unsigned int end_idx      {static_cast<unsigned int>(vec.size())-1};
    unsigned int idx_count          {0};
    for (auto& el : vec) {
        // dont add seperator for last element
        if (idx_count != end_idx) {
            to_return << el << sep;
        } else {
            to_return << el;
        }
        idx_count++;
    }
    return to_return.str();
}


}; // end of Helpers namespace
#endif