#include "Servo_Controller.h"

namespace RPI {
namespace gpio {
namespace Servo {

using std::cout;
using std::cerr;
using std::endl;

/********************************************* Static Defines *********************************************/

// min, neutral, max
const ServoAngles ServoController::YAW_ANGLES{2500, 1600, 500};
const ServoAngles ServoController::PITCH_ANGLES{1300, 1400, 2500};

/********************************************** Constructors **********************************************/


ServoController::ServoController(const std::uint8_t servo_i2c_addr)
    : PCA9685{servo_i2c_addr}
{
    // stub
}

ServoController::~ServoController() {
    // only cleanup if is init in first place
    // prevents client from trying to write to registers that do not exist
    if (ServoController::getIsInit()) {
        // make motors stop
        cout << "Resetting Servo Pins" << endl;
        // todo: convert this to single function
        if (SetServoPWM({{I2C_ServoAddr::YAW, 90}, {I2C_ServoAddr::PITCH, 90}}) != ReturnCodes::Success) {
            cerr << "Error: Failed to stop servos" << endl;
        }
    }
    setIsInit(false);
}

ReturnCodes ServoController::init() const {
    // if already init, stop now (have to specify whose getIsInit to call otherwise always true)
    if (ServoController::getIsInit()) return ReturnCodes::Success;

    if (PCA9685::init() != ReturnCodes::Success) {
        return ReturnCodes::Error;
    }

    if (SetServoPWM({{I2C_ServoAddr::PITCH, 90}, {I2C_ServoAddr::YAW, 90}}) != ReturnCodes::Success) {
        cerr << "Error: Failed to init servos to neutral position" << endl;
    }

    setIsInit(true);
    return ReturnCodes::Success;
}


/********************************************* Getters/Setters *********************************************/


/********************************************* Servo Functions *********************************************/

ReturnCodes ServoController::SetServoPWM(const I2C_ServoAddr sel_servo, const int angle) const {
    // try to convert angle to pwm signal
    // if fail/invalid dont try to use it
    std::optional<int> pwm_val {AngleToPwm(sel_servo, angle)};
    if (!pwm_val.has_value()) return ReturnCodes::Error;

    return PCA9685::SetPwm(static_cast<int>(sel_servo), 0, *pwm_val);
}

ReturnCodes ServoController::SetServoPWM(const ServoAnglePair pair) const {
    return SetServoPWM(pair.sel_servo, pair.angle);
}

ReturnCodes ServoController::SetServoPWM(const std::vector<ServoAnglePair> servo_angle_pairs) const {
    for (const auto& pair : servo_angle_pairs) {
        if (SetServoPWM(pair) != ReturnCodes::Success) {
            return ReturnCodes::Error;
        }
    }
    return ReturnCodes::Success;
}

/********************************************* Helper Functions ********************************************/

std::optional<int> ServoController::AngleToPwm(const I2C_ServoAddr sel_servo, const int angle) const {
    // make sure angle is within valid range (0-180)
    const int desired_angle     { std::max(180, std::min(0, angle)) };

    // the angles fraction of 180 degress (special case for 90)
    const bool  is_neutral      { angle == 90 }; 
    const float pwm_scale       { desired_angle / static_cast<float>(180) };

    // figure out which angles to be analyzing
    const std::optional<ServoAngles> angles {
        sel_servo == I2C_ServoAddr::YAW ?
            std::optional(YAW_ANGLES) : sel_servo == I2C_ServoAddr::PITCH ?
            std::optional(PITCH_ANGLES) : std::nullopt
    };

    if (!angles.has_value()) {
        cerr << "Error: Invalid Servo Selected" << endl;
        return std::nullopt;
    }

    // whatever angle comes out to be, have to translate it to a pwm pulse/duty cycle
    // https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf -- [age 25]
    // account for PWM frequency -- 1/prescale = 4096*Freq / MHz
    // note: 4096 = 2^12 from 12 bit reg
    // note: freq is in MHz (divide by Mega to get freq = # ticks)
    const auto  pwm_freq { PCA9685::GetPwmFreq() };
    float pwm_tick_mult {};
    if (pwm_freq.has_value()) {
        pwm_tick_mult = 4096 * (*pwm_freq) / 1000000;
    } else {
        cerr << "Error: pwm frequency not set" << endl;
        return std::nullopt;
    }

    // the angle converted to the servo's valid range 
    // examples:
    // angle = 0    -> range * 0 + min = min
    // angle = 90   -> neutral
    // angle = 180  -> range * 1 + min = range + min = max
    const int valid_angle {
        is_neutral ?
            angles->neutral :
            static_cast<int>(angles->range * pwm_scale) + angles->min
    };

    // finally return the angle multiplied by ticks to get duty cycle/pwm
    return pwm_tick_mult * valid_angle;
}


}; // end of Servo namespace
}; // end of gpio namespace
}; // end of RPI namespace
