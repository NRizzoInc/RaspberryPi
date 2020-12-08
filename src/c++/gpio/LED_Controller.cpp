#include "LED_Controller.h"

using std::cout;
using std::cerr;
using std::endl;


namespace gpio {
namespace LED {

/********************************************** Constructors **********************************************/
LEDController::LEDController()
    : color_to_leds ({
            {"red",     24}, // gpio24/pin 18 (can use variable brightness)
            {"yellow",  23},
            {"green",   18},
            {"blue",    15}
        })
{
    // init LEDs on board
    if(initLEDs() != ReturnCodes::Success) {
        cerr << "Failed to properly init LEDs" << endl;
    }
}

LEDController::~LEDController() {
    //stub
}

/********************************************* Public Helpers *********************************************/
std::vector<std::string> LEDController::getLedColorList() {
    return Helpers::getMapKeys(color_to_leds);
}



/********************************************* Helper Functions ********************************************/
ReturnCodes LEDController::initLEDs() {
    try {
        wiringPiSetup();
        return ReturnCodes::Success;
    } catch (std::exception err) {
        err.what();
        return ReturnCodes::Error;
    }
}


}; // end of LED namespace

}; // end of gpio namespace
