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
{
    cout << "Creating gpio obj" << endl;
}

GPIO_Controller::~GPIO_Controller() {
    // destructor stub
}

/********************************************* Public Helpers *********************************************/
std::vector<std::string> GPIO_Controller::getPairColorList() {
    return Helpers::getMapKeys(color_to_led_btn_pairs);
}



/********************************************* Helper Functions ********************************************/

// std::map<std::string, int> GPIO_Controller::generateLedBtnPairs() {}



}; // end of gpio namespace