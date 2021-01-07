#include "Motor_Controller.h"

namespace RPI {
namespace gpio {
namespace Motor {

using std::cout;
using std::cerr;
using std::endl;

/********************************************** Constructors **********************************************/
MotorController::MotorController(const std::uint8_t motor_i2c_addr)
    : PCA9685{motor_i2c_addr}
{
    // stub
}
MotorController::~MotorController() {
    // only cleanup if is init in first place
    // prevents client from trying to write to registers that do not exist
    if (MotorController::getIsInit()) {
        // make motors stop
        cout << "Resetting Motor Pins" << endl;
        if (SetMotorsPWM(0, 0, 0, 0) != ReturnCodes::Success) {
            cerr << "Error: Failed to stop motors" << endl;
        }
    }
    setIsInit(false);
}

ReturnCodes MotorController::init() const {
    // if already init, stop now (have to specify whose getIsInit to call otherwise always true)
    if (MotorController::getIsInit()) return ReturnCodes::Success;

    if (PCA9685::init() != ReturnCodes::Success) {
        return ReturnCodes::Error;
    }

    // once setup, make sure motors are stopped
    if(ChangeMotorDir(false, false, false, false) != ReturnCodes::Success) {
        cerr << "Failed to start motors at off position" << endl;
        return ReturnCodes::Error;
    }

    setIsInit(true);
    return ReturnCodes::Success;
}


/********************************************* Getters/Setters *********************************************/


/********************************************* Motor Functions *********************************************/

ReturnCodes MotorController::SetSingleMotorPWM(const I2C_MotorAddr motor_dir, const int duty) const {
    // see https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf -- page 10
    // each motor has to set the pwm in two place
    // based on input, the determine actual duty for the 2 pwm channels in reg that need to be set for the motor
    // each register has a channel for output and brightness control
    const int ch0 { static_cast<int>(motor_dir) };
    const int ch1 { ch0+1 };
    int duty0{};
    int duty1{};

    // strange case for back left wheel being opposite all other wheels
    const bool is_opposite { motor_dir == I2C_MotorAddr::BL };
    if (duty > 0) {
        duty0 = is_opposite ? 0         : duty;
        duty1 = is_opposite ? duty      : 0;
    } else if (duty < 0) {
        duty0 = is_opposite ? abs(duty) : 0;
        duty1 = is_opposite ? 0         : abs(duty);
    } else {
        // duty == 0 (stop)
        duty0 = 4095;
        duty1 = 4095;
    }

    if(SetPwm(ch0, 0, duty0) == ReturnCodes::Success && SetPwm(ch1, 0, duty1) == ReturnCodes::Success) {
        return ReturnCodes::Success;
    } else {
        return ReturnCodes::Error;
    }
}

ReturnCodes MotorController::ChangeMotorDir(
    const bool forward,
    const bool backward,
    const bool left,
    const bool right
) const {
    // default to none so dont have to waste extra if check for none
    Motor::YDirection vert     {YDirection::NONE};
    Motor::XDirection horiz   {XDirection::NONE};

    if (forward) {
        vert = YDirection::FORWARD;
    } else if (backward) {
        vert = YDirection::REVERSE;
    }

    if (left) {
        horiz = XDirection::LEFT;
    } else if (right) {
        horiz = XDirection::RIGHT;
    }

    return ChangeMotorDir(vert, horiz);
}


ReturnCodes MotorController::ChangeMotorDir(const YDirection vertical, const XDirection horizontal) const {
    // start off with medium duty (TODO: eventually add this as argument via another enum for slow, med, fast)
    
    // if backward, negate all
    // if not moving, set to 0
    const bool  is_forward      { vertical == YDirection::FORWARD };
    const bool  stopping        { vertical == YDirection::NONE };
    // vertical penalty (-1,0,1)
    const int   vert_pen        { stopping ? 0 : ( is_forward ? 1 : -1 ) };

    // if no horizontal component, multiply by 1
    // steer towards a side by having motors on that side negate
    const bool is_straight  { horizontal == XDirection::NONE };
    const bool is_right     { horizontal == XDirection::RIGHT };
    const int  duty_fl      { static_cast<int>(DUTY_MED * vert_pen * (is_straight ? 1 : (is_right ?  1 : -1) )) };
    const int  duty_fr      { static_cast<int>(DUTY_MED * vert_pen * (is_straight ? 1 : (is_right ? -1 :  1) )) };
    const int  duty_bl      { static_cast<int>(DUTY_MED * vert_pen * (is_straight ? 1 : (is_right ?  1 : -1) )) };
    const int  duty_br      { static_cast<int>(DUTY_MED * vert_pen * (is_straight ? 1 : (is_right ? -1 :  1) )) };
    return SetMotorsPWM(duty_fl, duty_fr, duty_bl, duty_br);
}


ReturnCodes MotorController::SetMotorsPWM(
    const int duty_fl,
    const int duty_fr,
    const int duty_bl,
    const int duty_br
) const {
    if (
        SetSingleMotorPWM(I2C_MotorAddr::FL, CheckDutyRange(duty_fl)) == ReturnCodes::Success &&
        SetSingleMotorPWM(I2C_MotorAddr::FR, CheckDutyRange(duty_fr)) == ReturnCodes::Success &&
        SetSingleMotorPWM(I2C_MotorAddr::BL, CheckDutyRange(duty_bl)) == ReturnCodes::Success &&
        SetSingleMotorPWM(I2C_MotorAddr::BR, CheckDutyRange(duty_br)) == ReturnCodes::Success
    ) {
        return ReturnCodes::Success;
    } else {
        return ReturnCodes::Error;
    }
}

void MotorController::testMotorsLoop(
    // not needed, but need to follow call guidlines for fn-mapping to work
    __attribute__((unused)) const std::vector<std::string>& colors,
    const unsigned int& interval,
    const int& duration,
    __attribute__((unused)) const unsigned int& rate
) const {
    cout << "Interval: " << interval << "ms" << endl;
    cout << "Duration: " << duration << "ms" << endl;

    // keep track of time/duration
    const auto start_time = std::chrono::steady_clock::now();

    while (
        !MotorController::getShouldThreadExit() &&
        // if duration == -1 : run forever
        (duration == -1 || Helpers::Timing::hasTimeElapsed(start_time, duration, std::chrono::milliseconds(1)))
    ) {
        // forward
        if (ChangeMotorDir(YDirection::FORWARD, XDirection::NONE) != ReturnCodes::Success) {
            cerr << "Error: Failed to move motors forward" << endl;
        } else {
            cout << "Moving forward" << endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        if (MotorController::getShouldThreadExit()) break;

        // back
        if (ChangeMotorDir(YDirection::REVERSE, XDirection::NONE) != ReturnCodes::Success) {
            cerr << "Error: Failed to move motors backward" << endl;
        }  else {
            cout << "Moving backward" << endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        if (MotorController::getShouldThreadExit()) break;

        // left
        if (ChangeMotorDir(YDirection::FORWARD, XDirection::LEFT) != ReturnCodes::Success) {
            cerr << "Error: Failed to move motors left" << endl;
        }  else {
            cout << "Moving left" << endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        if (MotorController::getShouldThreadExit()) break;

        // right
        if (ChangeMotorDir(YDirection::FORWARD, XDirection::RIGHT) != ReturnCodes::Success) {
            cerr << "Error: Failed to move motors right" << endl;
        }  else {
            cout << "Moving right" << endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
        if (MotorController::getShouldThreadExit()) break;

        // stop
        if (ChangeMotorDir(YDirection::NONE, XDirection::NONE) != ReturnCodes::Success) {
            cerr << "Error: Failed to stop motors" << endl;
        }  else {
            cout << "Stopping" << endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
}

/********************************************* Helper Functions ********************************************/

int MotorController::CheckDutyRange(const int duty) const {
    // range is [-4095, 4095]
    return std::max(std::min(duty, 4095), -4095);
}


}; // end of Motor namespace
}; // end of gpio namespace
}; // end of RPI namespace
