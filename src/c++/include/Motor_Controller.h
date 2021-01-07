#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <cmath>        // for abs
#include <algorithm>    // for max/min
#include <chrono>       // for setting sleep durations
#include <thread>       // for std::this_thread

// Our Includes
#include "constants.h"
#include "PCA9685_Interface.h"
#include "timing.hpp"

// 3rd Party Includes

namespace RPI {
namespace gpio {
namespace Motor {

// commonly used in this namespace
using Interface::XDirection;
using Interface::YDirection;

// Maps each tire/motor/servo to its i2c address
// note: each device has 2 channels (i.e. 0-1, 2-3, 4-5, 6-7)
enum class I2C_MotorAddr : int {
    FL_Motor          = 0,         // Front Left
    BL_Motor          = 2,         // Back  Left
    BR_Motor          = 4,         // Back  Right
    FR_Motor          = 6,         // Front Right   
}; // end of motor wheel addresses

constexpr int DUTY_MED         { 2000 };        // duty value for medium forward speed 
constexpr int DUTY_MED_BACK    { -DUTY_MED };   // duty value for medium backward speed

/**
 * @brief Handle class for I2C Chip for motors (PCA9685)
 * @note see https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf
 */
class MotorController : public gpio::Interface::PCA9685 {

    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a motor controller object responsible for handling motor interactions
         * @param motor_i2c_addr The address of the motor controller i2c board
         */
        MotorController(const std::uint8_t motor_i2c_addr);
        virtual ~MotorController();

        /**
         * @brief Helps intialize the motors
         * @return ReturnCodes
         */
        virtual ReturnCodes init() const;

        /********************************************* Getters/Setters *********************************************/

        /********************************************* Motor Functions *********************************************/

        /**
         * @brief Set a single motor's pwm with a desired duty cycle
         * @param motor_dir The specific motor to set
         * @param duty Higher Positives mean forward, Lower negatives mean backward
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes SetSingleMotorPWM(const I2C_MotorAddr motor_dir, const int duty) const;

        /**
         * @brief Easily facilitates changing direction by handling the changing of motor pwm signals
         * @param vertical Enum that defines robot's vertical vector
         * @param horizontal Enum that defines robot's horizontal vector
         * @note Treat car movement where +y = forward & +x = right (both NONE == stop)
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes ChangeMotorDir(const YDirection vertical, const XDirection horizontal) const;
        /**
         * @brief Overload to simplifies conversion of cardinal directions into enum
         * @param forward True if has a positive forward vector 
         * @param backward True if has a positive backward vector
         * @param left True if has a positive left vector
         * @param right True if has a positive right vector
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes ChangeMotorDir(const bool forward, const bool backward, const bool left, const bool right) const;

        /**
         * @brief Set all the motor's pwm with the desired duty cycle (higher level API: ChangeMotorDir())
         * @note Higher positive duty mean forward, lower negative duty mean backward
         * @param duty_fl Front Left Duty
         * @param duty_fr Front Right Duty
         * @param duty_bl Back Left Duty
         * @param duty_br Back Right Duty
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes SetMotorsPWM(
            const int duty_fl,
            const int duty_fr,
            const int duty_bl,
            const int duty_br
        ) const;

        /**
         * @brief Run the motors through a set pattern
         * @note Have to pass everything by reference do to function mapping requirements
         */
        void testMotorsLoop(
            // not needed, but need to follow call guidlines for fn-mapping to work
            __attribute__((unused)) const std::vector<std::string>& colors={},
            const unsigned int& interval=1000,
            const int& duration=-1,
            __attribute__((unused)) const unsigned int& rate=1
        ) const;

    private:
        /******************************************** Private Variables ********************************************/

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Converts the passed duty to a valid duty in the range (i.e. caps max/min)
         * @param duty The duty to make sure is in the valid range
         * @return std::uint8_t The capped off duty based on input
         */
        int CheckDutyRange(const int duty) const;

}; // MotorController



}; // end of Motor namespace
}; // end of gpio namespace
}; // end of RPI namespace


#endif