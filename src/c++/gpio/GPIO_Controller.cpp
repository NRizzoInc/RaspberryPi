#include "GPIO_Controller.h"

using std::cout;
using std::cerr;
using std::endl;

namespace gpio {

/********************************************** Constructors **********************************************/
GPIO_Controller::GPIO_Controller()
    // call constructors for parents
    : LED::LEDController()
    , Button::ButtonController()

    // init vars
    , color_to_led_btn_pairs ({
            //stub
        })
    , mode_to_action (std::move(createFnMap()))
{
    cout << "Creating gpio obj" << endl;
}

GPIO_Controller::~GPIO_Controller() {
    // destructor stub
}

/********************************************* Getters/Setters *********************************************/
std::vector<std::string> GPIO_Controller::getPairColorList() const {
    return Helpers::Map::getMapKeys(color_to_led_btn_pairs);
}

std::vector<std::string> GPIO_Controller::getModes() const {
    return Helpers::Map::getMapKeys(mode_to_action);
}



/********************************************* Helper Functions ********************************************/

// std::map<std::string, int> GPIO_Controller::generateLedBtnPairs() {}

Helpers::Map::FnMap GPIO_Controller::createFnMap() const {
    Helpers::Map::FnMap to_rtn;
    to_rtn.insert("Blink",      [](){ return &GPIO_Controller::blinkLEDs;   });
    to_rtn.insert("Intensity",  [](){ return &LEDController::LEDIntensity;  });
    return to_rtn;
}


}; // end of gpio namespace