#include "Motor_Controller.h"

namespace RPI {
namespace gpio {
namespace Motor {

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


/********************************************* Helper Functions ********************************************/


}; // end of Motor namespace
}; // end of gpio namespace
}; // end of RPI namespace
