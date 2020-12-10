#ifndef CONSTANTS_H
#define CONSTANTS_H

// Defines different possible returns rather than just success/fail
enum class ReturnCodes {
    Success,
    Error,
    TryAgain,
};

namespace Constants {
    // http://wiringpi.com/reference/software-pwm-library/
    const int LED_SOFT_PWM_MIN = 0;
    const int LED_SOFT_PWM_MAX = 100;
    const int LED_SOFT_PWM_RANGE = LED_SOFT_PWM_MAX - LED_SOFT_PWM_MIN;

}; // end of constants namespace

namespace CLI::Results {
    // shortening of parse results mapping
    using ParseResults = std::unordered_map<std::string, std::string>;

    const std::string MODE      { "mode"     };
    const std::string COLORS    { "names"    };
    const std::string INTERVAL  { "interval" };
    const std::string RATE      { "rate"     };
    const std::string DURATION  { "duration" };
}; // end of CLI::Results namespace

#endif