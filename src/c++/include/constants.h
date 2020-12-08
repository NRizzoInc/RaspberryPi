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

#endif