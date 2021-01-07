/**
 * @file print_enums.hpp
 * @brief Contains operator<< overrides for printing & using enums
 */
#ifndef PRINT_ENUMS_HPP
#define PRINT_ENUMS_HPP

// Standard Includes
#include <iostream>
#include <type_traits>

// use inline to not break "one definition rule"
template <typename T, typename = typename std::enable_if<std::is_enum<T>::value>::type>
inline std::ostream &operator<<(std::ostream &os, const T& enum_item) {
    return os << static_cast<typename std::underlying_type<T>::type>(enum_item);
}

#endif