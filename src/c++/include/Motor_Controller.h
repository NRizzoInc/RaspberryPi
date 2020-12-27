#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
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
    FR          = 2,         // Front Right
    BL          = 4,         // Back  Left
    BR          = 6,         // Back  Right
}; // end of motor wheel addresses

enum class I2C_PWM_Addr : std::uint8_t {
    MODE_REG    = 0x00,    // Root Register containing mode
    FREQ_REG    = 0xFE,    // Register for controlling the pwm frequency
    ON_LOW      = 0x06,    // Register for setting on duty cycle LOW pwm
    ON_HIGH     = 0x07,    // Register for setting on duty cycle HIGH pwm
    OFF_LOW     = 0x08,    // Register for setting off duty cycle LOW pwm
    OFF_HIGH    = 0x09,    // Register for setting off duty cycle HIGH pwm
}; // end of pwm addresses

/**
 * @brief Handle class for I2C Chip for motors (PCA9685)
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
         * @brief Set all the motor's pwm with the desired duty cycle
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

        const std::uint8_t motor_i2c_addr;               // the address for the robot's i2c motor module

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