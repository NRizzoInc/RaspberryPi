#ifndef MAP_HELPERS_HPP
#define MAP_HELPERS_HPP

// Standard Includes
#include <vector>
#include <map>
#include <unordered_map>
#include <functional>
#include <typeinfo>
#include <typeindex>
#include <cassert>

// Our Includes

// 3rd Party Includes

namespace Helpers {
namespace Map {

using voidFunctionType = std::function<void(void)>;

using FnMapType = std::unordered_map<
    std::string,
    std::pair<voidFunctionType, std::type_index>
>;

/**
 * @brief Helper class to map a string to a class's member functions
 * @note Instantiate => .insert(str, fnRef) => .searchAndCall<RtnType>(mapName, args)
 */
struct FnMap {
    // the mapping of str -> fn
    FnMapType str_to_fn;

    /**
     * @brief Insert a str-to-function pair in mapping
     * @param fn_str The string to map to a specific function
     * @param fn_ref The function the string should map to 
     */
    template<typename T>
    void insert(std::string fn_str, T fn_ref){
        auto tt = std::type_index(typeid(fn_ref));
        str_to_fn.insert(
            std::make_pair(
                fn_str,
                std::make_pair((voidFunctionType)fn_ref, tt)
            )
        );
    }

    /**
     * @brief Call & return from function mapped to be string
     * @param fn_str The string mapping to the function to call 
     * @param args The arguments to use when calling the function
     * @return T Whatever was specified as return of mapped function
     */
    template<typename T,typename... Args>
    T searchAndCall(std::string fn_str, Args&&... args){
        auto mapIter = str_to_fn.find(fn_str);
        /*chk if not end*/
        auto mapVal = mapIter->second;

        // auto typeCastedFun = reinterpret_cast<T(*)(Args ...)>(mapVal.first); 
        auto typeCastedFun = (T(*)(Args ...))(mapVal.first); 

        //compare the types is equal or not
        assert(mapVal.second == std::type_index(typeid(typeCastedFun)));
        return typeCastedFun(std::forward<Args>(args)...);
    }
};

/******************************************** Generic Helpers ********************************************/
// for ordered maps
template<typename keyType, typename valType>
inline std::vector<keyType> getMapKeys(const std::map<keyType, valType>& mapping) {
    std::vector<keyType> keys;
    for (auto& entry : mapping) {
        keys.push_back(entry.first);
    }
    return keys;
}

// for unordered maps
template<typename keyType, typename valType>
inline std::vector<keyType> getMapKeys(const std::unordered_map<keyType, valType>& mapping) {
    std::vector<keyType> keys;
    for (auto& entry : mapping) {
        keys.push_back(entry.first);
    }
    return keys;
}

// for function maps
inline std::vector<std::string> getMapKeys(const FnMap fnMapping) {
    std::vector<std::string> keys;
    for (auto& entry : fnMapping.str_to_fn) {
        keys.push_back(entry.first);
    }
    return keys;
}



}; // end of map namespace

/************************* Helpers namespace *************************/

template<class F, class... Args>
auto delay_invoke(F f, Args... args) {
    return [f=std::move(f), tup=std::make_tuple(std::move(args)...)]() -> decltype(auto) {
        return std::apply(f, tup);
    };
}

}; // end of helpers namespace

#endif
