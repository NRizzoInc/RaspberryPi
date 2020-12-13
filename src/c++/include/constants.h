#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <unordered_map>

// Defines different possible returns rather than just success/fail
enum class ReturnCodes {
    Success,
    Error,
    TryAgain,
};

namespace Constants {
    // http://wiringpi.com/reference/software-pwm-library/
    constexpr int LED_SOFT_PWM_MIN      {0};
    constexpr int LED_SOFT_PWM_MAX      {100};
    constexpr int LED_SOFT_PWM_RANGE    {LED_SOFT_PWM_MAX - LED_SOFT_PWM_MIN};

    constexpr std::size_t MAX_DATA_SIZE {512};

}; // end of constants namespace

namespace CLI::Results {
    // shortening of parse results mapping
    using ParseResults = std::unordered_map<std::string, std::string>;

    // cannot use std::string in constexpr (auto == char[])
    constexpr auto MODE      = "mode"       ;
    constexpr auto COLORS    = "names"      ;
    constexpr auto INTERVAL  = "interval"   ;
    constexpr auto RATE      = "rate"       ;
    constexpr auto DURATION  = "duration"   ;
    constexpr auto PORT      = "port"       ;
}; // end of CLI::Results namespace

#endif