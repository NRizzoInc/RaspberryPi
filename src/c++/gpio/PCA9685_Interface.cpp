#include "PCA9685_Interface.h"

namespace RPI {
namespace gpio {
namespace Interface {

using std::cout;
using std::cerr;
using std::endl;

/***************************************** Miscellaneous Helpers ******************************************/

std::ostream& operator<<(std::ostream& out, const gpio::Interface::I2C_PWM_Addr& addr) {
    return out << static_cast<int>(addr);
}

std::ostream& operator<<(std::ostream& out, const std::uint8_t& addr_8) {
    return out << static_cast<int>(addr_8);
}

/********************************************** Constructors **********************************************/

PCA9685::PCA9685()
    : GPIOBase{}
{
    // stub
}

PCA9685::~PCA9685() {
    // stub
}



/********************************************* Getters/Setters *********************************************/


/**************** PCA9685 Specific Functions (only utilized by those with direct need) *********************/


/********************************************* Helper Functions ********************************************/


}; // end of Interface namespace
}; // end of gpio namespace
}; // end of RPI namespace
