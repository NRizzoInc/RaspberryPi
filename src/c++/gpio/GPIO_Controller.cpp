#include "GPIO_Controller.h"

using std::cout;
using std::cerr;
using std::endl;

namespace gpio {

/********************************************** Constructors **********************************************/
GPIO_Controller::GPIO_Controller()
    : color_to_leds ({
            {"red",     24}, // gpio24/pin 18 (can use variable brightness)
            {"yellow",  23},
            {"green",   18},
            {"blue",    15}
        })
    , color_to_btns ({
            {"red",     2}, // gpio2/pin3
            {"yellow",  3},
            {"green",   4},
            {"blue",    17}
        })
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
std::vector<std::string> GPIO_Controller::getLedColorList() {
    return Helpers::getMapKeys(color_to_leds);
}

std::vector<std::string> GPIO_Controller::getBtnColorList() {
    return Helpers::getMapKeys(color_to_btns);
}

std::vector<std::string> GPIO_Controller::getPairColorList() {
    return Helpers::getMapKeys(color_to_led_btn_pairs);
}



/********************************************* Helper Functions ********************************************/

// std::map<std::string, int> GPIO_Controller::generateLedBtnPairs() {}



}; // end of gpio namespace