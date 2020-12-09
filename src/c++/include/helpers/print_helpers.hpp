
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

}; // end of helpers namespace

#endif
