#include "Button_Controller.h"

using std::cout;
using std::cerr;
using std::endl;


namespace gpio {
namespace Button {

/********************************************** Constructors **********************************************/
ButtonController::ButtonController()
    : color_to_btns ({
            {"red",     2}, // gpio2/pin3
            {"yellow",  3},
            {"green",   4},
            {"blue",    17}
        })
    , isInit(false)
{
    // stub
}

ButtonController::~ButtonController() {
    if (getIsInit()) {
        cout << "Resetting Button Pins" << endl;
        setIsInit(false);
    }
}

ReturnCodes ButtonController::init() const {
    // if already init, stop now
    if (getIsInit()) return ReturnCodes::Success;

    // setup pins for their purpose
    if (wiringPiSetup() == -1) {
        return ReturnCodes::Error;
    }

    // TODO: Init button input pins

    setIsInit(true);
    return ReturnCodes::Success;
}

/********************************************* Getters/Setters *********************************************/
std::vector<std::string> ButtonController::getBtnColorList() const {
    return Helpers::Map::getMapKeys(color_to_btns);
}

bool ButtonController::getIsInit() const {
    return isInit;
}

ReturnCodes ButtonController::setIsInit(const bool new_state) const {
    isInit = new_state;
    return ReturnCodes::Success;
}

ReturnCodes ButtonController::setShouldThreadExit(const bool new_status) const {
    stop_thread = new_status;
    return ReturnCodes::Success;
}

const std::atomic_bool& ButtonController::getShouldThreadExit() const {
    return stop_thread;
}

/******************************************** Button Functions ********************************************/


/********************************************* Helper Functions ********************************************/

}; // end of Button namespace

}; // end of gpio namespace
