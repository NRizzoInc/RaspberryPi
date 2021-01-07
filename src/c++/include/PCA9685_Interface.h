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

// 3rd Party Includes
#include <wiringPi.h>
#include <softPwm.h>
#include <wiringPiI2C.h>

#include "Servo_Controller.h"

namespace RPI {
namespace gpio {
namespace Interface {

// Maps each tire/motor to its i2c address
/// note: each motor has 2 channels (i.e. 0-1, 2-3, 4-5, 6-7)
enum class I2C_Addr : int {
    FL_Motor          = 0,         // Front Left
    BL_Motor          = 2,         // Back  Left
    BR_Motor          = 4,         // Back  Right
    FR_Motor          = 6,         // Front Right
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

// handle cout with enum (cannot print uint8_t bc alias for char* so prints ascii)
std::ostream& operator<<(std::ostream& out, const gpio::Interface::I2C_PWM_Addr& addr);
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

        /********************************************* Getters/Setters *********************************************/

        virtual bool getIsInit() const override;

    protected:
        /**************** PCA9685 Specific Functions (only utilized by those with direct need) *********************/

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

    private:
        /******************************************** Private Variables ********************************************/

        // static vars because should only be initialized once for all derived classes
        static std::optional<std::uint8_t>  PCA9685_i2c_addr;    // the address of robot's i2c PCA9685 module
        static int                          PCA9685_i2c_fd;      // file descriptor created by setup
        static bool                         is_PCA9685_init;     // true if PCA9685 device has been init properly

        /********************************************* Helper Functions ********************************************/

}; // end of PCA9685 class

}; // end of Interface namespace
}; // end of gpio namespace
}; // end of RPI namespace


#endif
