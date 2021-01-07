#include "PCA9685_Interface.h"

namespace RPI {
namespace gpio {
namespace Interface {

using std::cout;
using std::cerr;
using std::endl;

/***************************************** Miscellaneous Helpers ******************************************/

std::ostream& operator<<(std::ostream& out, const PCA9685_Reg_Addr& addr) {
    return out << static_cast<int>(addr);
}

std::ostream& operator<<(std::ostream& out, const std::uint8_t& addr_8) {
    return out << static_cast<int>(addr_8);
}

/********************************************** Constructors **********************************************/

// init static vars
unsigned int                    PCA9685::PCA9685_init_count{0};             // start from 0
std::optional<std::uint8_t>     PCA9685::PCA9685_i2c_addr{std::nullopt};    // default to 0x40 (most likely is this)
int                             PCA9685::PCA9685_i2c_fd{-1};                // invalid
std::optional<float>            PCA9685::pwm_freq{std::nullopt};            // unset/invalid

PCA9685::PCA9685(const std::optional<std::uint8_t> PCA9685_i2c_addr)
    : GPIOBase{}
{
    if (!PCA9685::PCA9685_i2c_addr.has_value()) {
        if (PCA9685_i2c_addr.has_value()) {
            // no address yet & user passed one, set it
            PCA9685::PCA9685_i2c_addr = *PCA9685_i2c_addr;
        } else {
            // no address yet, so use default
            PCA9685::PCA9685_i2c_addr = 0x40;
        }
    } else if (!PCA9685::getIsInit() && PCA9685_i2c_addr.has_value()) {
        // if already have address, but not yet init (i.e. might be wrong) can set new val
        PCA9685::PCA9685_i2c_addr = *PCA9685_i2c_addr;
    }
}

PCA9685::~PCA9685() {
    // only cleanup if is init in first place
    // prevents client from trying to write to registers that do not exist
    if (PCA9685::getIsInit()) {
        // if PCA9685 device fd is open, close it and set to -1
        cout << "Resetting PCA9685 Device" << endl;
        if (PCA9685_i2c_fd > 0) {
            close(PCA9685_i2c_fd);
            PCA9685_i2c_fd = -1;
        }
        setIsInit(false);
    }
}


ReturnCodes PCA9685::init() const {
    // if already init, stop now (have to specify whose getIsInit to call otherwise always true)
    if (PCA9685::getIsInit()) return ReturnCodes::Success;

    // setup pins for their purpose
    PCA9685_i2c_fd = wiringPiI2CSetup(*PCA9685_i2c_addr);
    if (PCA9685_i2c_fd == -1) {
        cerr << "Error: Failed to init I2C PCA9685 Module" << endl;
        return ReturnCodes::Error;
    }

    if(SetPwmFreq(50.0) != ReturnCodes::Success) {
        cerr << "Error: Failed to set I2C PCA9685 Module's PWM Frequency" << endl;
        return ReturnCodes::Error;
    }

    setIsInit(true);
    return ReturnCodes::Success;
}


/********************************************* Getters/Setters *********************************************/

bool PCA9685::getIsInit() const {
    // both have to be init for PCA9685 to be ready
    return PCA9685::PCA9685_init_count > 0 && GPIOBase::getIsInit();
}

ReturnCodes PCA9685::setIsInit(const bool new_state) const {
    PCA9685::PCA9685_init_count += new_state ? 1 : -1; // if setting to false, decrement
    return GPIOBase::setIsInit(new_state);
}


/**************** PCA9685 Specific Functions (only utilized by those with direct need) *********************/


ReturnCodes PCA9685::WriteReg(const std::uint8_t reg_addr, const std::uint8_t data) const {
    ReturnCodes rtn {wiringPiI2CWriteReg8(
        PCA9685_i2c_fd,
        reg_addr,
        data
    ) < 0 ? ReturnCodes::Error : ReturnCodes::Success};

    if (rtn != ReturnCodes::Success) {
        // have to print as int (uint8_t maps to ascii char)
        cerr << "Error: Failed to write to register @" << std::hex << reg_addr << endl;
    }
    return rtn;
}

std::uint8_t PCA9685::ReadReg(const std::uint8_t reg_addr) const {
    return wiringPiI2CReadReg8(PCA9685_i2c_fd, static_cast<int>(reg_addr));
}

ReturnCodes PCA9685::SetPwmFreq(const float freq) const {
    // instr: https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf
    // -- page 15: how to set
    // -- page 25: how to calc scaled freq

    // scale frequency
    float prescaleval {25000000.0};     // 25MHz
    prescaleval /= 4096.0;              // 12-bit
    prescaleval /= freq;                // 25MHz/50 = .5MHz
    prescaleval -= 1.0;
    const int scaled_freq = floor(prescaleval + .5); // round

    // reset mode & get default settings
    const std::uint8_t mode_reg  { static_cast<std::uint8_t>(PCA9685_Reg_Addr::MODE_REG) };
    if(WriteReg(mode_reg, 0) != ReturnCodes::Success) {
        cerr << "Failed to reset mode register" << endl;
    }
    
    // pause pwm freq register to update it
    const std::uint8_t oldmode   { ReadReg(mode_reg) };                                   // rewrite after update
    const std::uint8_t newmode   { static_cast<std::uint8_t>((oldmode & 0x7F) | 0x10) };  // sleep (bit4 & restart off)
    const ReturnCodes  sleep_rtn { WriteReg(mode_reg, newmode) };                         // go to sleep
    if(sleep_rtn != ReturnCodes::Success) {
        cerr << "Failed to put pwm freq register to sleep" << endl;
        return sleep_rtn;
    }

    // update pwm freq
    if(WriteReg(static_cast<std::uint8_t>(PCA9685_Reg_Addr::FREQ_REG), scaled_freq) != ReturnCodes::Success) {
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
    pwm_freq = freq;
    return ReturnCodes::Success;
}

std::optional<float> PCA9685::GetPwmFreq() const {
    return pwm_freq;
}


ReturnCodes PCA9685::SetPwm(const int channel, const int on, const int off) const {
    // arduino, but same idea: https://learn.adafruit.com/16-channel-pwm-servo-driver?view=all#using-as-gpio-2980401-5
    // see https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf -- page 16
    // have to update all pwm registers
    // each motor channel has 1 of each pwm registers (hence the 4*channel to get the correct address)

    // helper function to get the address based on base address
    auto calc_addr = [&](const PCA9685_Reg_Addr base_addr){
        return static_cast<std::uint8_t>(
            static_cast<std::uint8_t>(base_addr) +
            static_cast<std::uint8_t>(4*channel) // 4 pwm regs per channel
        );
    };

    if (WriteReg(calc_addr(PCA9685_Reg_Addr::ON_LOW_BASE),  on & 0xFF) != ReturnCodes::Success) {
        cerr << "Failed to update ON LOW PWM" << endl;
        return ReturnCodes::Error;
    }

    if (WriteReg(calc_addr(PCA9685_Reg_Addr::ON_HIGH_BASE), on >> 8) != ReturnCodes::Success) {
        cerr << "Failed to update ON HIGH PWM" << endl;
        return ReturnCodes::Error;
    }

    if (WriteReg(calc_addr(PCA9685_Reg_Addr::OFF_LOW_BASE), off & 0xFF) != ReturnCodes::Success) {
        cerr << "Failed to update OFF LOW PWM" << endl;
        return ReturnCodes::Error;
    }

    if (WriteReg(calc_addr(PCA9685_Reg_Addr::OFF_HIGH_BASE), off >> 8) != ReturnCodes::Success) {
        cerr << "Failed to update OFF HIGH PWM" << endl;
        return ReturnCodes::Error;
    }

    return ReturnCodes::Success;
}

/********************************************* Helper Functions ********************************************/


}; // end of Interface namespace
}; // end of gpio namespace
}; // end of RPI namespace
