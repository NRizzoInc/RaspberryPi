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

void LEDController::blinkLEDs(std::vector<std::string> colors, unsigned int interval) {
    for (auto& to_blink : colors) {
        // on
        digitalWrite(color_to_leds.at(to_blink), HIGH);
        delay(interval);
        // off
        digitalWrite(color_to_leds.at(to_blink), LOW);
        delay(interval);
    }
}


/********************************************* Helper Functions ********************************************/
ReturnCodes LEDController::initLEDs() {
    // setup pins for their purpose
    wiringPiSetup();
    for (auto& led_entry : color_to_leds) {
        // set each led pin as an output
        pinMode(led_entry.second, OUTPUT);
    }
    return ReturnCodes::Success;
}


}; // end of LED namespace

}; // end of gpio namespace
