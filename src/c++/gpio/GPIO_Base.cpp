#include "GPIO_Base.h"

using std::cout;
using std::cerr;
using std::endl;


namespace gpio {

/********************************************** Constructors **********************************************/
GPIOBase::GPIOBase()
    : isInit{false}
    , stop_thread{false}
{
    // stub
}

GPIOBase::~GPIOBase() {
    // stub
}

/********************************************* Getters/Setters *********************************************/


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

}; // end of gpio namespace
