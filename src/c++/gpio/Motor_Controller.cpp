#include "Motor_Controller.h"

namespace RPI {
namespace gpio {
namespace Motor {

using std::cout;
using std::cerr;
using std::endl;

/********************************************** Constructors **********************************************/
MotorController::MotorController()
    : GPIOBase{}
    , motor_i2c_addr{0x40} // TODO: make this a cli arg (gotten with `i2cdetect -y 1`)
{
    // stub
}
MotorController::~MotorController() {
    // stub
}

ReturnCodes MotorController::init() const {
    // if already init, stop now
    if (getIsInit()) return ReturnCodes::Success;

    // setup pins for their purpose
    if (wiringPiI2CSetup (motor_i2c_addr) == -1) {
        return ReturnCodes::Error;
    }

    setIsInit(true);
    return ReturnCodes::Success;
}


/********************************************* Getters/Setters *********************************************/

/********************************************* Motor Functions *********************************************/

ReturnCodes MotorController::SetSingleMotorPWM(const I2C_Addr motor_dir, const int duty) const {
    // each motor has to set the pwm in two place
    // based on input, the determine actual duty for the 2 channels that need to be set for the motor
    const int ch0 {static_cast<int>(motor_dir)};
    const int ch1 {static_cast<int>(motor_dir)+1};
    int duty0{};
    int duty1{};

    if (duty > 0) {
        duty0 = 0;
        duty1 = duty;
    } else if (duty < 0) {
        duty0 = abs(duty);
        duty1 = 0;
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

ReturnCodes MotorController::SetMotorsPWM(
    const int duty_fl,
    const int duty_fr,
    const int duty_bl,
    const int duty_br
) const {
    if (
        SetSingleMotorPWM(I2C_Addr::FL, CheckDutyRange(duty_fl)) == ReturnCodes::Success &&
        SetSingleMotorPWM(I2C_Addr::FR, CheckDutyRange(duty_fr)) == ReturnCodes::Success &&
        SetSingleMotorPWM(I2C_Addr::BL, CheckDutyRange(duty_bl)) == ReturnCodes::Success &&
        SetSingleMotorPWM(I2C_Addr::BR, CheckDutyRange(duty_br)) == ReturnCodes::Success
    ) {
        return ReturnCodes::Success;
    } else {
        return ReturnCodes::Error;
    }
}


/********************************************* Helper Functions ********************************************/

ReturnCodes MotorController::WriteReg(const std::uint8_t reg_addr, const std::uint8_t data) const {
    return wiringPiI2CWriteReg8(
        motor_i2c_addr,
        reg_addr,
        data
    ) < 0 ? ReturnCodes::Error : ReturnCodes::Success;
}

std::uint8_t MotorController::ReadReg(const std::uint8_t reg_addr) const {
    return wiringPiI2CReadReg8(motor_i2c_addr, static_cast<int>(reg_addr));
}

ReturnCodes MotorController::SetPwm(const int channel, const int on, const int off) const {
    // have to update all pwm registers
    // each motor channel has 1 of each pwm registers (hence the 4*channel to get the correct address)

    if (WriteReg(static_cast<std::uint8_t>(I2C_PWM_Addr::ON_LOW) + 4*channel, on & 0xFF) != ReturnCodes::Success) {
        cerr << "Failed to update ON LOW PWM" << endl;
        return ReturnCodes::Error;
    }

    if (WriteReg(static_cast<std::uint8_t>(I2C_PWM_Addr::ON_HIGH) + 4*channel, on >> 8) != ReturnCodes::Success) {
        cerr << "Failed to update ON HIGH PWM" << endl;
        return ReturnCodes::Error;
    }

    if (WriteReg(static_cast<std::uint8_t>(I2C_PWM_Addr::OFF_LOW) + 4*channel, off & 0xFF) != ReturnCodes::Success) {
        cerr << "Failed to update OFF LOW PWM" << endl;
        return ReturnCodes::Error;
    }

    if (WriteReg(static_cast<std::uint8_t>(I2C_PWM_Addr::OFF_HIGH ) +4*channel,  off >> 8) != ReturnCodes::Success) {
        cerr << "Failed to update OFF HIGH PWM" << endl;
        return ReturnCodes::Error;
    }

    return ReturnCodes::Success;
}

int MotorController::CheckDutyRange(const int duty) const {
    // range is [-4095, 4095]
    return std::max(std::min(duty, 4095), -4095);
}


}; // end of Motor namespace
}; // end of gpio namespace
}; // end of RPI namespace
