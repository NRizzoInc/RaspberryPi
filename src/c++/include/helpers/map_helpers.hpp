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
#include <initializer_list>

// Our Includes

// 3rd Party Includes

namespace Helpers {
namespace Map {

// each stored function will have this type (which will be reinterpretted when needed)
typedef void (*voidFunctionType)(void);

// class member function pointer
template <typename classType>
using voidClassMemFnType = void(classType::*)();

// the map's type to map a string to a class's member functions
template <typename classType>
using ClassFnMapType = std::unordered_map<
    std::string,
    voidClassMemFnType<classType>
>;

/**
 * @brief Helper class to map a string to a class's member functions (maps str -> fn)
 * @note Instantiate => .insert(str, fnRef) => .searchAndCall<RtnType>(mapName, args)
 */
template <typename classType>
class ClassFnMap : public ClassFnMapType<classType> {

public:
    ClassFnMap() : ClassFnMapType<classType>() {}

    /**
     * @brief Insert a str-to-function pair in mapping
     * @param fn_str The string to map to a specific function
     * @param fn_ref The function the string should map to 
     */
    template<typename T>
    void insert(const std::string& fn_str, T fn_ref) {
        ClassFnMapType<classType>::insert(
            std::make_pair(
                fn_str,
                (voidClassMemFnType<classType>)fn_ref
                // std::make_pair(static_cast<voidFunctionType>(fn_ref), tt)
            )
        );
    }

    /************************ How to Call Inserted Member Functions *************************/


    /********************************* Const Function Calls *********************************/
    /**
     * @brief Call & return from function mapped to the string (const ref to obj)
     * @param class_inst An instantiated object of same class type used for member function mapping
     * @param fn_str The string mapping to the function to call 
     * @param args The arguments to use when calling the function
     * @return T Whatever was specified as return of mapped function
     * @note Usage: my_FnMap.searchAndCall<return type>(fn_str, args)
     */
    template<typename T,typename... Args>
    T searchAndCall(const classType& class_inst, const std::string& fn_str, Args&&... args) const {
        auto map_val = ClassFnMapType<classType>::at(fn_str);

        // add const to function to signify it will not modify contents of obj 
        auto typeCastedMemberFunc = reinterpret_cast<T(classType::*)(Args ...) const>(map_val);

        // actually call the function
        return (class_inst.*typeCastedMemberFunc)(std::forward<Args>(args)...);
    }

    /********************************* Non-Const Function Calls *********************************/
    /**
     * @brief Call & return from function mapped to the string (const ref to obj)
     * @param class_inst An instantiated object of same class type used for member function mapping
     * @param fn_str The string mapping to the function to call 
     * @param args The arguments to use when calling the function
     * @return T Whatever was specified as return of mapped function
     * @note Usage: my_FnMap.searchAndCall<return type>(fn_str, args)
     * This is for non-const functions
     */
    template<typename T,typename... Args>
    T searchAndCall(classType& class_inst, const std::string& fn_str, Args&&... args) {
        auto map_val = ClassFnMapType<classType>::at(fn_str);

        // add const to function to signify it will not modify contents of obj 
        auto typeCastedMemberFunc = reinterpret_cast<T(classType::*)(Args ...)>(map_val);

        // actually call the function
        return (class_inst.*typeCastedMemberFunc)(std::forward<Args>(args)...);
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
template <typename classType>
inline std::vector<std::string> getMapKeys(const ClassFnMap<classType> fnMapping) {
    std::vector<std::string> keys;
    for (auto& entry : fnMapping) {
        keys.push_back(entry.first);
    }
    return keys;
}



}; // end of map namespace


}; // end of helpers namespace

#endif
