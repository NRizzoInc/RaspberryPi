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
    , motor_i2c_fd{-1}     // invalid
{
    // stub
}
MotorController::~MotorController() {
    // if motor device fd is open, close it and set to -1
    if (motor_i2c_fd > 0) {
        close(motor_i2c_fd);
        motor_i2c_fd = -1;
    }
}

ReturnCodes MotorController::init() const {
    // if already init, stop now (have to specify whose getIsInit to call otherwise always true)
    if (MotorController::getIsInit()) return ReturnCodes::Success;

    // setup pins for their purpose
    motor_i2c_fd = wiringPiI2CSetup(motor_i2c_addr);
    if (motor_i2c_fd == -1) {
        cerr << "Error: Failed to init I2C Motor Module" << endl;
        return ReturnCodes::Error;
    }

    if(SetPwmFreq(50.0) != ReturnCodes::Success) {
        cerr << "Error: Failed to set I2C Motor Module's PWM Frequency" << endl;
        return ReturnCodes::Error;
    } 

    setIsInit(true);
    return ReturnCodes::Success;
}


/********************************************* Getters/Setters *********************************************/

std::ostream& operator<<(std::ostream& out, const I2C_PWM_Addr& addr) {
    return out << static_cast<int>(addr);
}

std::ostream& operator<<(std::ostream& out, const std::uint8_t& addr_8) {
    return out << static_cast<int>(addr_8);
}


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

void MotorController::testLoop(
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
        if (SetMotorsPWM(2000, 2000, 2000, 2000) != ReturnCodes::Success) {
            cerr << "Error: Failed to move motors forward" << endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

        // // back
        // if (SetMotorsPWM(-2000, -2000, -2000, -2000) != ReturnCodes::Success) {
        //     cerr << "Error: Failed to move motors backward" << endl;
        // }
        // std::this_thread::sleep_for(std::chrono::milliseconds(interval));

        // // left
        // if (SetMotorsPWM(-500, -500, 2000, 2000) != ReturnCodes::Success) {
        //     cerr << "Error: Failed to move motors left" << endl;
        // }
        // std::this_thread::sleep_for(std::chrono::milliseconds(interval));

        // // right
        // if (SetMotorsPWM(2000, 2000, -500, -500) != ReturnCodes::Success) {
        //     cerr << "Error: Failed to move motors right" << endl;
        // }
        // std::this_thread::sleep_for(std::chrono::milliseconds(interval));

        // // stop
        // if (SetMotorsPWM(0, 0, 0, 0) != ReturnCodes::Success) {
        //     cerr << "Error: Failed to stop motors" << endl;
        // }
        // std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
}

/********************************************* Helper Functions ********************************************/

ReturnCodes MotorController::WriteReg(const std::uint8_t reg_addr, const std::uint8_t data) const {
    ReturnCodes rtn {wiringPiI2CWriteReg8(
        motor_i2c_fd,
        reg_addr,
        data
    ) < 0 ? ReturnCodes::Error : ReturnCodes::Success};

    if (rtn != ReturnCodes::Success) {
        // have to print as int (uint8_t maps to ascii char)
        cerr << "Error: Failed to write to register @" << std::hex << reg_addr << endl;
    }
    return rtn;
}

std::uint8_t MotorController::ReadReg(const std::uint8_t reg_addr) const {
    return wiringPiI2CReadReg8(motor_i2c_fd, static_cast<int>(reg_addr));
}

ReturnCodes MotorController::SetPwmFreq(const float freq) const {
    // instr: https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf -- page 15

    // scale frequency
    float prescaleval {25000000.0};     // 25MHz
    prescaleval /= 4096.0;              // 12-bit
    prescaleval /= freq;                // 25MHz/50 = .5MHz
    prescaleval -= 1.0;
    const int scaled_freq = floor(prescaleval + .5); // round

    // get old freq & pause pwm freq register to update it
    const std::uint8_t mode_reg  { static_cast<std::uint8_t>(I2C_PWM_Addr::MODE_REG) };
    const std::uint8_t oldmode   { ReadReg(mode_reg) };                                   // rewrite after update
    const std::uint8_t newmode   { static_cast<std::uint8_t>((oldmode & 0x7F) | 0x10) };  // sleep (bit4 & restart off)
    const ReturnCodes  sleep_rtn { WriteReg(mode_reg, newmode) };                         // go to sleep
    if(sleep_rtn != ReturnCodes::Success) {
        cerr << "Failed to put pwm freq register to sleep" << endl;
        return sleep_rtn;
    }

    // update pwm freq
    if(WriteReg(static_cast<std::uint8_t>(I2C_PWM_Addr::FREQ_REG), scaled_freq) != ReturnCodes::Success) {
        cerr << "Failed to update pwm freq" << endl;
        return ReturnCodes::Error;
    }

    // restore old mode
    if(WriteReg(mode_reg, oldmode) != ReturnCodes::Success) {
        cerr << "Failed to restore to original mode from sleep" << endl;
        return ReturnCodes::Error;
    }

    // wait a bit for interrupt to pick up change & set mode to appropriate final setting
    std::this_thread::sleep_for(std::chrono::microseconds(500));
    if(WriteReg(mode_reg, oldmode | 0x80) != ReturnCodes::Success) {
        cerr << "Failed to set mode to final setting" << endl;
        return ReturnCodes::Error;
    }

    // success
    return ReturnCodes::Success;
}


ReturnCodes MotorController::SetPwm(const int channel, const int on, const int off) const {
    // see https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf -- page 16
    // have to update all pwm registers
    // each motor channel has 1 of each pwm registers (hence the 4*channel to get the correct address)

    // helper function to get the address based on base address
    auto calc_addr = [&](const I2C_PWM_Addr base_addr){
        return static_cast<std::uint8_t>(
            static_cast<std::uint8_t>(base_addr) +
            static_cast<std::uint8_t>(4*channel) // 4 pwm regs per channel
        );
    };

    if (WriteReg(calc_addr(I2C_PWM_Addr::ON_LOW_BASE),  on & 0xFF) != ReturnCodes::Success) {
        cerr << "Failed to update ON LOW PWM" << endl;
        return ReturnCodes::Error;
    }

    if (WriteReg(calc_addr(I2C_PWM_Addr::ON_HIGH_BASE), on >> 8) != ReturnCodes::Success) {
        cerr << "Failed to update ON HIGH PWM" << endl;
        return ReturnCodes::Error;
    }

    if (WriteReg(calc_addr(I2C_PWM_Addr::OFF_LOW_BASE), off & 0xFF) != ReturnCodes::Success) {
        cerr << "Failed to update OFF LOW PWM" << endl;
        return ReturnCodes::Error;
    }

    if (WriteReg(calc_addr(I2C_PWM_Addr::OFF_HIGH_BASE), off >> 8) != ReturnCodes::Success) {
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
