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
{
    // init Buttons on board
    if(initButtons() != ReturnCodes::Success) {
        cerr << "Failed to properly init buttons" << endl;
    }
}

ButtonController::~ButtonController() {
    //stub
}

/********************************************* Getters/Setters *********************************************/
std::vector<std::string> ButtonController::getBtnColorList() {
    return Helpers::getMapKeys(color_to_btns);
}


/******************************************** Button Functions ********************************************/


/********************************************* Helper Functions ********************************************/
ReturnCodes ButtonController::initButtons() {
    try {
        wiringPiSetup();
        return ReturnCodes::Success;
    } catch (std::exception err) {
        err.what();
        return ReturnCodes::Error;
    }
}

}; // end of Button namespace

}; // end of gpio namespace
