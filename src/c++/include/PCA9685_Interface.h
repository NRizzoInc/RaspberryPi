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


}; // end of Interface namespace
}; // end of gpio namespace
}; // end of RPI namespace


#endif
