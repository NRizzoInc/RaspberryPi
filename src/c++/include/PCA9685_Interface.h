#ifndef PCA9685_INTERFACE_H
#define PCA9685_INTERFACE_H
// This file is responsible for managing the PCA9685 chip which controls the servos and motors

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
#include <optional>

// Our Includes
#include "constants.h"
#include "GPIO_Base.h"
#include "timing.hpp"
#include "enum_helpers.hpp"

// 3rd Party Includes
#include <wiringPi.h>
#include <softPwm.h>
#include <wiringPiI2C.h>

namespace RPI {
namespace gpio {
namespace Interface {

// register addresses for the I2C Chip for motors (PCA9685)
// https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf -- page 10
// ON/OFF_LOW/HIGH address = Base registers + 4 * motor#(0-3)
enum class PCA9685_Reg_Addr : std::uint8_t {
    MODE_REG        = 0x00,    // Root Register containing mode
    ON_LOW_BASE     = 0x06,    // Register for setting on duty cycle LOW pwm
    ON_HIGH_BASE    = 0x07,    // Register for setting on duty cycle HIGH pwm
    OFF_LOW_BASE    = 0x08,    // Register for setting off duty cycle LOW pwm
    OFF_HIGH_BASE   = 0x09,    // Register for setting off duty cycle HIGH pwm
    FREQ_REG        = 0xFE,    // Register for controlling the pwm frequency
}; // end of pwm addresses

/**
 * @brief Enum which defines possible Y directions the robot can move
 */
enum class YDirection : int {
    REVERSE         = -1,    // moving towards backward
    FORWARD         =  1,    // moving towards forward
    NONE            =  0     // if not moving 
}; // end of XDirection

/**
 * @brief Enum which defines possible X directions the robot can move
 */
enum class XDirection : int {
    LEFT            = -1,    // moving towards left
    RIGHT           =  1,    // moving towards right
    NONE            =  0     // if moving straight forward/back
}; // end of XDirection

// handle cout with enum (cannot print uint8_t bc alias for char* so prints ascii)
std::ostream& operator<<(std::ostream& out, const gpio::Interface::PCA9685_Reg_Addr& addr);
std::ostream& operator<<(std::ostream& out, const std::uint8_t& addr_8);


class PCA9685 : public GPIOBase {

    public:
        /********************************************** Constructors ***********************************************/

        /**
         * @brief Construct the interface dealing with the i2c PCA9685 device
         * responsible for handling motor/servo interactions
         * @param PCA9685_i2c_addr The address of the PCA9685 i2c device on the gpio board
         */
        PCA9685(
            const std::optional<std::uint8_t> PCA9685_i2c_addr=std::nullopt
        );
        virtual ~PCA9685();

        /**
         * @brief Helps intialize the PCA9685 device by setting PWM freq & other prerequisites
         * @return ReturnCodes
         */
        virtual ReturnCodes init() const;

        /**
         * @brief Cleans up utilized resources, should be called in derived destructors
         */
        virtual void cleanup() const;

        /********************************************* Getters/Setters *********************************************/

        virtual bool getIsInit() const override;

        /// @note this should only be called by derived classed 
        virtual ReturnCodes setIsInit(const bool new_state) const override;

    protected:
        static constexpr float MAX_PWM {4096.0};   // the max possible pwm signal that can be set (12-bit)

        /**************** PCA9685 Specific Functions (only utilized by those with direct need) *********************/

        /**
         * @brief Write data to a register in the Motor's i2c device
         * @param reg_addr The specific motor to write to (based on PCA9685_Reg_Addr enum mapping to addresses)
         * @param data The data to write
         * @return ReturnCodes 
         */
        ReturnCodes WriteReg(const std::uint8_t reg_addr, const std::uint8_t data) const;
        ReturnCodes WriteReg(const PCA9685_Reg_Addr reg_addr, const std::uint8_t data) const;

        /**
         * @brief Read data from a register in the Motor's i2c device
         * @param reg_addr The specific motor to read from (based on PCA9685_Reg_Addr enum mapping to addresses)
         * @return The found data
         */
        std::uint8_t ReadReg(const std::uint8_t reg_addr) const;

        /**
         * @brief Sets the pwm signal's frequency
         * @param freq (defaults to 50Hz) The frequnecy of the pwm signal in Hz
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes SetPwmFreq(const float freq=50.0) const;

        /**
         * @brief Get the pwm frequency (in Hz)
         * @return The pwm frequency currently being used
         */
        std::optional<float> GetPwmFreq() const;

        /**
         * @brief Convert the device's pwm frequency to # ticks
         * @return The period/ticks/cycles in ms
         */
        std::optional<float> GetPwmPeriod() const;

        /**
         * @brief Sets the pwm duty cycle for a motor (changes the speed/direction of the motor)
         * @param channel The motor's channel
         * @param on On time
         * @param off Off time
         * @return ReturnCodes Success if no issues 
         */
        ReturnCodes SetPwm(const int channel, const int on, const int off) const;

        /**
         * @brief Enables or deactivates full-on
         * @param channel The channel/pin to enable/disable full turn on for
         * @param enable true: full-on
         * @param enable false: disable and proceed according to PWM signals
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes TurnFullOn(const int channel, const bool enable) const;

        /**
         * @brief Enables or deactivates full-of
         * @param channel The channel/pin to enable/disable full turn off for
         * @param enable true: full-off
         * @param enable false: disable and proceed according to PWM signals (or full on if set)
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes TurnFullOff(const int channel, const bool enable) const;

    private:
        /******************************************** Private Variables ********************************************/

        // static vars because should only be initialized once for all derived classes
        static std::optional<std::uint8_t>  PCA9685_i2c_addr;   // the address of robot's i2c PCA9685 module
        static int                          PCA9685_i2c_fd;     // file descriptor created by setup
        static unsigned int                 PCA9685_init_count; // keep track so can be "deinit" that many times
        static std::optional<float>         pwm_freq;           // the pwm frequency the device is set to (in Hz)

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Helper function that calculates the base address for a channel's specific register 
         * @param base_addr The register that whose address is needed relative to the channel
         * @param channel The channel whose register address is needed
         * @return The address
         */
        std::uint8_t CalcChBaseAddr(const PCA9685_Reg_Addr base_addr, const int channel) const;


}; // end of PCA9685 class

}; // end of Interface namespace
}; // end of gpio namespace
}; // end of RPI namespace


#endif
