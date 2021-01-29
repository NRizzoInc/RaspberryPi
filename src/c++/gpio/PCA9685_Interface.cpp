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

PCA9685::PCA9685(const std::optional<std::uint8_t> PCA9685_i2c_addr, const bool verbosity)
    : GPIOBase{verbosity}
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
    // since this is base class, it will be the first to destruct
    // (problematic bc want to close the fd after derived classes reset)
    // derived classes should call cleanup() instead
}


ReturnCodes PCA9685::init() const {
    // if already init, stop now (have to specify whose getIsInit to call otherwise always true)
    if (PCA9685::getIsInit()) return ReturnCodes::Success;

    // check if valid rpi first
    if(GPIOBase::init() != ReturnCodes::Success) {
        return ReturnCodes::Error;
    }

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

    return ReturnCodes::Success;
}

void PCA9685::cleanup() const {
    // workaround for fact that this destructor will be called prior to derived destructors
    // who still need access to the i2c file descriptor (close when last one is in about to exit)
    if (PCA9685_init_count == 1 && PCA9685_i2c_fd > 0) {
        cout << "Resetting PCA9685 Device" << endl;
        close(PCA9685_i2c_fd);
        PCA9685_i2c_fd = -1;
    }
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

ReturnCodes PCA9685::WriteReg(const PCA9685_Reg_Addr reg_addr, const std::uint8_t data) const {
    return WriteReg(static_cast<std::uint8_t>(reg_addr), data);
}

ReturnCodes PCA9685::WriteReg(const std::uint8_t reg_addr, const std::uint8_t data) const {

    // if fd not open, dont try to write
    if (PCA9685_i2c_fd == -1) return ReturnCodes::Success;

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
    prescaleval /= PCA9685::MAX_PWM;    
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
    if(WriteReg(PCA9685_Reg_Addr::FREQ_REG, scaled_freq) != ReturnCodes::Success) {
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

std::optional<float> PCA9685::GetPwmPeriod() const {
    // freq is in hz, so result will be in ticks/sec but decimal
    // convert to ms to not deal with floats (mult by 1000)
    // i.e. period(sec) =    1 / freq 
    // i.e. period(ms)  = 1000 / freq
    if (auto pwm_freq = GetPwmFreq()) {
        return *pwm_freq > 0 ? 1000 / *pwm_freq : 0;
    } else {
        cerr << "Error: pwm frequency not set" << endl;
        return std::nullopt;

    }
}


ReturnCodes PCA9685::SetPwm(const int channel, const int on, const int off) const {
    // arduino, but same idea: https://learn.adafruit.com/16-channel-pwm-servo-driver?view=all#using-as-gpio-2980401-5
    // see https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf -- page 16
    // have to update all pwm registers

    if (WriteReg(CalcChBaseAddr(PCA9685_Reg_Addr::ON_LOW_BASE, channel),  on & 0xFF) != ReturnCodes::Success) {
        if(isVerbose()) cerr << "Failed to update ON LOW PWM" << endl;
        return ReturnCodes::Error;
    }

    if (WriteReg(CalcChBaseAddr(PCA9685_Reg_Addr::ON_HIGH_BASE, channel), on >> 8) != ReturnCodes::Success) {
        if(isVerbose()) cerr << "Failed to update ON HIGH PWM" << endl;
        return ReturnCodes::Error;
    }

    if (WriteReg(CalcChBaseAddr(PCA9685_Reg_Addr::OFF_LOW_BASE, channel), off & 0xFF) != ReturnCodes::Success) {
        if(isVerbose()) cerr << "Failed to update OFF LOW PWM" << endl;
        return ReturnCodes::Error;
    }

    if (WriteReg(CalcChBaseAddr(PCA9685_Reg_Addr::OFF_HIGH_BASE, channel), off >> 8) != ReturnCodes::Success) {
        if(isVerbose()) cerr << "Failed to update OFF HIGH PWM" << endl;
        return ReturnCodes::Error;
    }

    return ReturnCodes::Success;
}

ReturnCodes PCA9685::TurnFullOn(const int channel, const bool enable) const {
    // get current on state to modify specific bits
    const int on_reg_addr   { CalcChBaseAddr(PCA9685_Reg_Addr::ON_HIGH_BASE, channel) };
    const int curr_on_state { ReadReg(on_reg_addr) };

    // set bit 4 to 0/1 (disabled/enabled)
    const std::uint8_t new_on_state = enable ? (curr_on_state | 0x10) : (curr_on_state & 0xEF);

    // write new settings (if enabling, than have to disable full off as well)
    WriteReg(PCA9685_Reg_Addr::ON_HIGH_BASE, new_on_state);
    if (enable) TurnFullOff(channel, false);

    return ReturnCodes::Success;
}

ReturnCodes PCA9685::TurnFullOff(const int channel, const bool enable) const {
    // get current off state to modify specific bits
    const int off_reg_addr      { CalcChBaseAddr(PCA9685_Reg_Addr::OFF_HIGH_BASE, channel) };
    const int curr_off_state    { ReadReg(off_reg_addr) };

    // set bit 4 to 0/1 (disabled/enabled)
    const std::uint8_t new_off_state = enable ? (curr_off_state | 0x10) : (curr_off_state & 0xEF);

    // write new settings
    WriteReg(off_reg_addr, new_off_state);

    return ReturnCodes::Success;
}

/********************************************* Helper Functions ********************************************/

std::uint8_t PCA9685::CalcChBaseAddr(const PCA9685_Reg_Addr base_addr, const int channel) const {
    // each servo/motor channel has 1 of each pwm registers (hence the 4*channel to get the correct address)
    return static_cast<std::uint8_t>(
        static_cast<std::uint8_t>(base_addr) +
        static_cast<std::uint8_t>(4*channel) // 4 pwm regs per channel
    );
}



}; // end of Interface namespace
}; // end of gpio namespace
}; // end of RPI namespace
