#ifndef MAP_HELPERS_HPP
#define MAP_HELPERS_HPP

// Standard Includes
#include <vector>
#include <map>

// Our Includes

// 3rd Party Includes

namespace Helpers {

template<typename keyType, typename valType>
inline std::vector<keyType> getMapKeys(const std::map<keyType, valType>& mapping) {
    std::vector<keyType> keys;
    for (auto& entry : mapping) {
        keys.push_back(entry.first);
    }
    return keys;
}

}; // end of helpers namespace

#endif
