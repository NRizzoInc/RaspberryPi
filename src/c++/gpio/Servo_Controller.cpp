#include "Servo_Controller.h"

namespace RPI {
namespace gpio {
namespace Servo {

using std::cout;
using std::cerr;
using std::endl;

/********************************************* Static Defines *********************************************/

/********************************************** Constructors **********************************************/


ServoController::ServoController(const std::uint8_t servo_i2c_addr)
    : PCA9685{servo_i2c_addr}
    , pos{{
        // init servos to neutral position (aka 90 degrees)
        {I2C_ServoAddr::YAW, 90},
        {I2C_ServoAddr::PITCH, 90}
    }}
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
        if (TurnServosOff() != ReturnCodes::Success) {
            cerr << "Error: Failed to turn off servos" << endl;
        }
        cleanup();
    }
    setIsInit(false);
}

ReturnCodes ServoController::init() const {
    // if already init, stop now (have to specify whose getIsInit to call otherwise always true)
    if (ServoController::getIsInit()) return ReturnCodes::Success;

    if (PCA9685::init() != ReturnCodes::Success) {
        return ReturnCodes::Error;
    }

    // starts servos at 90 degrees (neutral)
    if (SetServoPWM({{I2C_ServoAddr::PITCH, 90}, {I2C_ServoAddr::YAW, 90}}) != ReturnCodes::Success) {
        cerr << "Error: Failed to init servos to neutral position" << endl;
    }

    setIsInit(true);
    return ReturnCodes::Success;
}


/********************************************* Getters/Setters *********************************************/


/********************************************* Servo Functions *********************************************/

ReturnCodes ServoController::SetServoPWM(const I2C_ServoAddr sel_servo, const int angle) const {
    // try to convert angle to pwm signal (if success, udpate current state)
    ReturnCodes rtn = SetPwm(static_cast<int>(sel_servo), 0, AngleToPwmDuty(angle));
    if (rtn == ReturnCodes::Success) {
        pos[sel_servo] = angle;
    }
    return rtn;
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

ReturnCodes ServoController::TurnServosOff() const {
    // have to make sure to turn ON mode off, and OFF mode on
    bool rtn_succ {true};
    rtn_succ &= TurnFullOff(static_cast<int>(I2C_ServoAddr::PITCH), true) == ReturnCodes::Success;
    rtn_succ &= TurnFullOn(static_cast<int>(I2C_ServoAddr::PITCH), false) == ReturnCodes::Success;
    rtn_succ &= TurnFullOff(static_cast<int>(I2C_ServoAddr::YAW), true) == ReturnCodes::Success;
    rtn_succ &= TurnFullOn(static_cast<int>(I2C_ServoAddr::YAW), false) == ReturnCodes::Success;
    return rtn_succ ? ReturnCodes::Success : ReturnCodes::Error;
}


/********************************************* Helper Functions ********************************************/

float ServoController::ScaleAnglePercDuty(const int angle) const {
    // scale [0-180] -> [1.0-2.0]
    // make angle stay between 0-180
    // ex: assume freq = 50Hz (aka period = 20ms)
    // 0   degrees: 1.0 ms duty period (5%   duty)
    // 90  degrees: 1.5 ms duty period (7.5% duty)
    // 180 degrees: 2.0 ms duty period (10%  duty)
    // hence, min=5%, max=10% and scale between them
    constexpr int   ANGLE_MIN           {0};
    constexpr int   ANGLE_MAX           {180};
    constexpr float DUTY_MIN_PERC       {.05}; // 5%
    constexpr float DUTY_MAX_PERC       {.10}; // 10%
    constexpr float DUTY_RANGE          {DUTY_MAX_PERC-DUTY_MIN_PERC};

    // perform calcs to scale angle to % duty cycle
    // multiply percent against the actual period to get the final answer
    const int   valid_angle {std::max(ANGLE_MIN, std::min(ANGLE_MAX, angle))};
    const float perc_angle  {static_cast<float>(valid_angle) / static_cast<float>(ANGLE_MAX)};
    const float perc_duty   {DUTY_RANGE*perc_angle + DUTY_MIN_PERC};
    return perc_duty;
}


int ServoController::AngleToPwmDuty(const int angle) const {
    // whatever duty cycle/period/ticks comes out to be, have to scale it with the max allowed PWM signal
    // https://learn.adafruit.com/adafruits-raspberry-pi-lesson-8-using-a-servo-motor/servo-motors
    // https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf -- [age 25]

    // get the duty cycle (max possible pwm * percentage on)
    const float perc_duty {ScaleAnglePercDuty(angle)};
    return PCA9685::MAX_PWM * perc_duty;
}


}; // end of Servo namespace
}; // end of gpio namespace
}; // end of RPI namespace
