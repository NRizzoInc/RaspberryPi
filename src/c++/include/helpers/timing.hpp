#ifndef TIMING_HPP
#define TIMING_HPP

// Standard Includes
#include <chrono>   // clock
#include <ctime>    // localtime
#include <iomanip>  // put_time
#include <sstream>  // stringstream

// Our Includes

// 3rd Party Includes

namespace Helpers::Timing {

constexpr auto timecode_repr {"%Y-%m-%dT%H:%M:%S"};

/**
 * @brief Determines if the required amount of time has passed since start
 * @param start_time The time the clock started
 * @param duration How long this should go for (returns true at this point)
 * @return true Enough time has elapsed
 * @return false Not enough time has elapsed
 */
inline bool hasTimeElapsed(
    const std::chrono::_V2::steady_clock::time_point& start_time,
    const std::chrono::steady_clock::duration duration
) {
    const auto end_time = std::chrono::steady_clock::now();
    const auto elapsed_time {end_time - start_time};
    return elapsed_time > duration;
}


/**
 * @brief Converts the time to ISO 8601 standard
 * @param time (defaults to current time) The time to convert
 * @return stringified version of the ISO 8601 timecode
 */
inline std::string GetTimecode(
    const std::chrono::_V2::system_clock::time_point time=std::chrono::system_clock::now()
) {
    auto in_time_t {std::chrono::system_clock::to_time_t(time)};
    std::stringstream time_str;
    time_str << std::put_time(
        std::localtime(&in_time_t),
        timecode_repr // TODO: add ms?
    );

    return time_str.str();
}

}; // end of Helpers::Timing namespace

#endif
