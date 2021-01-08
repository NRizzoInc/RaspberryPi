#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <cmath>        // for abs
#include <algorithm>    // for max/min
#include <chrono>       // for setting sleep durations
#include <thread>       // for std::this_thread
#include <optional>
#include <unordered_map>

// Our Includes
#include "constants.h"
#include "PCA9685_Interface.h"
#include "timing.hpp"

// 3rd Party Includes

namespace RPI {
namespace gpio {
namespace Servo {

// commonly used in this namespace
using Interface::XDirection;
using Interface::YDirection;

// Maps each tire/motor/servo to its i2c address
// note robot has 8 total possible servo slots (but only 2 are used) 
enum class I2C_ServoAddr : int {
    YAW         =  8,         // Controls sideways camera servo
    PITCH       =  9,         // Controls vertical camera servo
    UNUSED_1    = 10,
    UNUSED_2    = 11,
    UNUSED_3    = 12,
    UNUSED_4    = 13,
    UNUSED_5    = 14,
    UNUSED_6    = 15,
}; // end of servo addresses

// contains the address (aka the servo to move) and the angle to move it to
struct ServoAnglePair {
    /**
     * @param sel_servo The selected servo
     * @param angle the angle to move the selected servo
     */
    ServoAnglePair(const I2C_ServoAddr sel_servo, const int angle)
        : sel_servo{sel_servo}
        , angle{angle}
        {}
    const I2C_ServoAddr sel_servo;
    const int angle;
}; // end of ServoAnglePair

/**
 * Handle class for I2C Chip for servos (PCA9685)
 * 
 * see https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf
 * Servo pwm math based on duty cycle percentages:
 * 0   degrees: 5%
 * 90  degrees: 7.5%
 * 180 degrees: 10%
 */
class ServoController : public gpio::Interface::PCA9685 {

    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a servo controller object responsible for handling servo interactions
         * @param servo_i2c_addr The address of the servo controller i2c board
         */
        ServoController(const std::uint8_t servo_i2c_addr);
        virtual ~ServoController();

        /**
         * @brief Helps intialize the servos
         * @return ReturnCodes
         */
        virtual ReturnCodes init() const;

        /********************************************* Getters/Setters *********************************************/

        /********************************************* Servo Functions *********************************************/

        /**
         * @brief Set a single servo's pwm with a desired duty cycle (final endpoint for overloads)
         * @param sel_servo The specific servo to set/move
         * @param angle The position to move the servo to
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes SetServoPWM(const I2C_ServoAddr sel_servo, const int angle) const;
        ReturnCodes SetServoPWM(const ServoAnglePair) const;
    
        /**
         * @brief Sets all servos' pwm with a desired duty cycle
         * @param servo_angle_pairs vector pairs of servos & where to move them to
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes SetServoPWM(const std::vector<ServoAnglePair> servo_angle_pairs) const;

        /**
         * @brief Responsible for turning off the servos so they do not stay primed past the duration of the program
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes TurnServosOff() const;

        /**
         * @brief Run the servos through a set pattern or have them move to a specific location and stop
         * @note Have to pass everything by reference do to function mapping requirements
         */
        void testServos(
            // not needed, but need to follow call guidlines for fn-mapping to work
            __attribute__((unused)) const std::vector<std::string>& colors={},
            const unsigned int& interval=1000,
            const int& duration=-1,
            __attribute__((unused)) const unsigned int& rate=1
        ) const;

    private:
        /******************************************** Private Variables ********************************************/

        mutable std::unordered_map<I2C_ServoAddr, int> pos;     // maps the servo's to their current positions (angles)

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Convert the easily understandable angle to the servo's corresponding pwm value
         * @param angle The angle to check if is valid and covnert to pwm value for the servo [0-180]
         * @return Integer representing the pwm value to use
         */
        int AngleToPwmDuty(const int angle) const;

        /**
         * @brief Scales angle to be in valid range & into a duty cycle (multiply by period to get # ticks)
         * @param angle The angle to scale
         * @return The percentage of time the duty cycle is "on"
         */
        float ScaleAnglePercDuty(const int angle) const;

}; // ServoController

}; // end of Servo namespace
}; // end of gpio namespace
}; // end of RPI namespace


#endif
