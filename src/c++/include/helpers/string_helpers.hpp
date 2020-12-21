#ifndef STRING_HELPERS_HPP
#define STRING_HELPERS_HPP

// Standard Includes
#include <string>
#include <sstream>
#include <vector>
#include <algorithm>

namespace Helpers {

/// if printing a pair
template<typename keyType, typename valType>
std::ostream& operator<<(std::ostream& out, const std::pair<keyType, valType>& pair) {
    return out << pair.first << ": " << pair.second; 
}

/**
 * @brief Splits a string into multiple elements within a vector
 * @param delim What to split by (not included in produced vectors' elements)
 * @param to_split The string to split
 * @return The split string in the form of a vector
 */
inline const std::vector<std::string> splitStr(const char delim, const std::string to_split) {
    std::vector<std::string> split_str;

    // iterate over string (backwards bc of push_back)
    // check edge case of empty string (dont add to vector)
    std::string::size_type beg = 0;
    for (std::size_t end = 0; (end = to_split.find(delim, end)) != std::string::npos; ++end) {
        const std::string& el = to_split.substr(beg, end - beg);
        if (el.size() > 0) {
            split_str.push_back(el);
        }
        beg = end + 1;
    }
    const std::string& final_el {to_split.substr(beg)};
    if (final_el.size() > 0) {
        split_str.push_back(final_el);
    }

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


/**
 * @brief Determines if a string contains a substring
 * @param str The string to search through
 * @param substr The substring to look for
 * @return true str contains substr
 * @return false str does not contain substr
 */
inline bool contains(const std::string& str, const std::string& substr) {
    return str.find(substr) != std::string::npos;
}

/**
 * @brief Determines if a string ends with a substring
 * @param str The string to search
 * @param ending The substring to look for at the end of str
 * @return true if str ends with ending
 * @return false if str does not end with ending
 */
inline bool endsWith(const std::string& str, const std::string& ending) {
    if (str.length() >= ending.length()) {
        return (0 == str.compare(str.length() - ending.length(), ending.length(), ending));
    } else {
        return false;
    }
}

/**
 * @brief Determines if a string starts with a substring
 * @param str The string to search
 * @param start The substring to look for at the start of str
 * @return true if str starts with start
 * @return false if str does not start with start
 */
inline bool startsWith(const std::string& str, const std::string& start) {
    return str.rfind(start, 0) != std::string::npos;
}

/**
 * @brief Determines the string that comes after the last '/'
 * @param str The string to parse
 * @return The substring after the last '/'
 */
inline std::string findFilename(const std::string& str) {
    return str.substr(str.find_last_of("/"));
}

}; // end of Helpers namespace

#endif