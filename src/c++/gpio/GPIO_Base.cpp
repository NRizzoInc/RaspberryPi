#include "GPIO_Base.h"

using std::cout;
using std::cerr;
using std::endl;

namespace RPI {

namespace gpio {


/******************************************* Static Member Init ********************************************/
std::optional<bool> GPIOBase::is_valid_pi {std::nullopt};

/********************************************** Constructors **********************************************/
GPIOBase::GPIOBase(const bool verbosity)
    : is_verbose{verbosity}
    , isInit{false}
    , stop_thread{false}
{
    // stub
}

GPIOBase::~GPIOBase() {
    // stub
}

ReturnCodes GPIOBase::init() const {
    if (!isValidRPI()) {
        cerr << "Error: Not a valid RPI... forgoing setup" << endl;
        return ReturnCodes::Error;
    }

    if (wiringPiSetup() == -1) {
        return ReturnCodes::Error;
    }

    return ReturnCodes::Success;
}


/********************************************* Getters/Setters *********************************************/

bool GPIOBase::isVerbose() const {
    return is_verbose;
}

ReturnCodes GPIOBase::setShouldThreadExit(const bool new_status) const {
    stop_thread.store(new_status);
    return ReturnCodes::Success;
}

bool GPIOBase::getShouldThreadExit() const {
    return stop_thread.load();
}

bool GPIOBase::getIsInit() const {
    return isInit;
}

ReturnCodes GPIOBase::setIsInit(const bool new_state) const {
    isInit = new_state;
    return ReturnCodes::Success;
}

/****************************************** Basic Board Functions ******************************************/

// credit: inspiration from wiringPi.c's `piGpioLayout()`
bool GPIOBase::isValidRPI() const {
    // if already called, dont try again
    if (is_valid_pi.has_value()) return *is_valid_pi;

    // default to invalid (set to true if found)
    static bool found_line {false};

    // scan file for hardware line (if dne, then not a pi and setup will fail)
    std::ifstream info_file{"/proc/cpuinfo"};
    std::string line_buf {};
    while (getline (info_file, line_buf)) {
        // if found, set to true
        if(Helpers::startsWith(line_buf, "Hardware")) {
            found_line = true;
        }
    }

    info_file.close();

    is_valid_pi = found_line;
    return found_line;
}


}; // end of gpio namespace

}; // end of RPI namespace
