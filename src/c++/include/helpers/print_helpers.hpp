
#ifndef PRINT_HELPERS_HPP
#define PRINT_HELPERS_HPP

// Standard Includes
#include <sstream>
#include <vector>

// Our Includes

// 3rd Party Includes

namespace Helpers {



/**
 * @brief Creates a string containing all elements of the vector (comma-seperated)
 * @param colors 
 * @return std::string 
 */
template <typename vecType>
inline std::string createVecStr(const std::vector<vecType>& vec, const bool space_sep=true) {
    std::stringstream to_return;
    for (auto& el : vec) {
        const std::string space_char = space_sep ? " " : "";
        to_return << el << "," << space_char;
    }
    return to_return.str();
}

}; // end of helpers namespace

#endif
