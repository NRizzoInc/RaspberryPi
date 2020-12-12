#ifndef TIMING_HPP
#define TIMING_HPP

// Standard Includes
#include <chrono>

// Our Includes

// 3rd Party Includes

namespace Helpers::Timing {

/**
 * @brief Determines if the required amount of time has passed since start
 * @param start_time The time the clock started
 * @param duration How long this should go for (returns true at this point)
 * @param time_unit The chrono time unit of measure (i.e. std::chrono::milliseconds(1))
 * @return true Enough time has elapsed
 * @return false Not enough time has elapsed
 */
template<typename timeUnit>
bool hasTimeElapsed(
    const std::chrono::_V2::steady_clock::time_point& start_time,
    const int duration,
    __attribute__((unused)) const timeUnit time_unit // dont need besides as way to get unit
) {
    const auto end_time = std::chrono::steady_clock::now();
    const auto elapsed_time = std::chrono::duration_cast<timeUnit>(end_time - start_time).count();
    return elapsed_time > duration;
}

}; // end of Helpers::Timing namespace

#endif
