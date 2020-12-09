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
    isInit = false;
}

ReturnCodes ButtonController::init() {
    // if already init, stop now
    if (isInit) return ReturnCodes::Success;

    // setup pins for their purpose
    if (wiringPiSetup() == -1) {
        return ReturnCodes::Error;
    }

    // TODO: Init button input pins

    isInit = true;
    return ReturnCodes::Success;
}

/********************************************* Getters/Setters *********************************************/
std::vector<std::string> ButtonController::getBtnColorList() {
    return Helpers::Map::getMapKeys(color_to_btns);
}

bool ButtonController::getIsInit() const {
    return isInit;
}

/******************************************** Button Functions ********************************************/


/********************************************* Helper Functions ********************************************/

}; // end of Button namespace

}; // end of gpio namespace
