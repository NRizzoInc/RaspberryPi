/**
 * @file math_helpers.hpp
 */
#ifndef MATH_HELPERS_HPP
#define MATH_HELPERS_HPP

// Standard Includes
#include <iostream>
#include <type_traits>

namespace Helpers::Math {

// use inline to not break "one definition rule"
template <typename T>
inline T negate(const T val) {
    return val*-1;
}


} // namespace Helpers::Math


#endif
