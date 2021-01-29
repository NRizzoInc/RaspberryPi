#include "Ultrasonic.h"


// to keep lines short
using std::cout;
using std::cerr;
using std::endl;


namespace RPI {
namespace gpio {
namespace Ultrasonic {

/********************************************** Constructors **********************************************/

DistSensor::DistSensor(const bool verbosity)
    : GPIOBase{verbosity}
{
    // stub
}

DistSensor::~DistSensor() {
    // set all ultrasonic sensor's pins to off at end
    if (getIsInit()) {

        setIsInit(false);
    }
}

ReturnCodes DistSensor::init() const {
    // if already init, stop now
    if (getIsInit()) return ReturnCodes::Success;

    if(GPIOBase::init() != ReturnCodes::Success) {
        return ReturnCodes::Error;
    }

    // TODO: setup pins

    setIsInit(true);
    return ReturnCodes::Success;
}

/********************************************* Getters/Setters *********************************************/


/****************************************** Ultrasonic Functions *******************************************/



/********************************************* Helper Functions ********************************************/


}; // end of Ultrasonic namespace
}; // end of gpio namespace
}; // end of RPI namespace
