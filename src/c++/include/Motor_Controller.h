#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unistd.h>     // for close fd
#include <cstdint>      // for std::uint8_t
#include <cmath>        // for abs
#include <algorithm>    // for max/min
#include <chrono>
#include <thread>

// Our Includes
#include "constants.h"
#include "GPIO_Base.h"
#include "timing.hpp"

// 3rd Party Includes
#include <wiringPi.h>
#include <softPwm.h>
#include <wiringPiI2C.h>

namespace RPI {
namespace gpio {
namespace Motor {

// Maps each tire/motor to its i2c address
/// note: each motor has 2 channels (i.e. 0-1, 2-3, 4-5, 6-7)
enum class I2C_Addr : int {
    FL          = 0,         // Front Left
    BL          = 2,         // Back  Left
    BR          = 4,         // Back  Right
    FR          = 6,         // Front Right
}; // end of motor wheel addresses

// register addresses for the I2C Chip for motors (PCA9685)
// https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf -- page 10
// ON/OFF_LOW/HIGH address = Base registers + 4 * motor#(0-3)
enum class I2C_PWM_Addr : std::uint8_t {
    MODE_REG        = 0x00,    // Root Register containing mode
    ON_LOW_BASE     = 0x06,    // Register for setting on duty cycle LOW pwm
    ON_HIGH_BASE    = 0x07,    // Register for setting on duty cycle HIGH pwm
    OFF_LOW_BASE    = 0x08,    // Register for setting off duty cycle LOW pwm
    OFF_HIGH_BASE   = 0x09,    // Register for setting off duty cycle HIGH pwm
    FREQ_REG        = 0xFE,    // Register for controlling the pwm frequency
}; // end of pwm addresses

/**
 * @brief Enum which defines possible vertical directions the robot can move
 */
enum class VertDir : int {
    REVERSE         =-1,    // moving towards backward
    FORWARD         = 1,    // moving towards forward
    NONE            = 0     // if not moving 
}; // end of HorizDir

/**
 * @brief Enum which defines possible horizontal directions the robot can move
 */
enum class HorizDir : int {
    LEFT            =-1,    // moving towards left
    RIGHT           = 1,    // moving towards right
    NONE            = 0     // if moving straight forward/back
}; // end of HorizDir

constexpr int DUTY_MED         { 2000 };        // duty value for medium forward speed 
constexpr int DUTY_MED_BACK    { -DUTY_MED };   // duty value for medium backward speed

// handle cout with enum (cannot print uint8_t bc alias for char* so prints ascii)
std::ostream& operator<<(std::ostream& out, const I2C_PWM_Addr& addr);
std::ostream& operator<<(std::ostream& out, const std::uint8_t& addr_8);

/**
 * @brief Handle class for I2C Chip for motors (PCA9685)
 * @note see https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf
 */
class MotorController : public GPIOBase {

    public:
        /********************************************** Constructors **********************************************/
        MotorController();
        virtual ~MotorController();

        /**
         * @brief Helps intialize the leds
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
        ReturnCodes SetSingleMotorPWM(const I2C_Addr motor_dir, const int duty) const;

        /**
         * @brief Easily facilitates changing direction by handling the changing of motor pwm signals
         * @param vertical Enum that defines robot's vertical vector
         * @param horizontal Enum that defines robot's horizontal vector
         * @note Treat car movement where +y = forward & +x = right (both NONE == stop)
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes ChangeMotorDir(const VertDir vertical, const HorizDir horizontal) const;

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
        void testLoop(
            // not needed, but need to follow call guidlines for fn-mapping to work
            __attribute__((unused)) const std::vector<std::string>& colors={},
            const unsigned int& interval=1000,
            const int& duration=-1,
            __attribute__((unused)) const unsigned int& rate=1
        ) const;

    private:
        /******************************************** Private Variables ********************************************/

        const   std::uint8_t    motor_i2c_addr;     // the address for the robot's i2c motor module
        mutable int             motor_i2c_fd;       // file descriptor created by setup (mutable for const init)

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Write data to a register in the Motor's i2c device
         * @param reg_addr The specific motor to write to (based on I2C_PWM_Addr enum mapping to addresses)
         * @param data The data to write
         * @return ReturnCodes 
         */
        ReturnCodes WriteReg(const std::uint8_t reg_addr, const std::uint8_t data) const;

        /**
         * @brief Read data from a register in the Motor's i2c device
         * @param reg_addr The specific motor to read from (based on I2C_PWM_Addr enum mapping to addresses)
         * @return The found data
         */
        std::uint8_t ReadReg(const std::uint8_t reg_addr) const;

        /**
         * @brief Sets the pwm signal's frequency
         * @param freq (defaults to 50MHz) The frequnecy of the pwm signal in MHz
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes SetPwmFreq(const float freq=50.0) const;

        /**
         * @brief Sets the pwm duty cycle for a motor (changes the speed/direction of the motor)
         * @param channel The motor's channel
         * @param on On time
         * @param off Off time
         * @return ReturnCodes Success if no issues 
         */
        ReturnCodes SetPwm(const int channel, const int on, const int off) const;

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