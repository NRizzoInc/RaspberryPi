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
    , mode_to_action (createFnMap())
{
    // stub
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


bool GPIO_Controller::getIsInit() const {
    return LEDController::getIsInit() && ButtonController::getIsInit();
}


/*********************************************** GPIO Helpers **********************************************/

ReturnCodes GPIO_Controller::init() const {
    // immediately return if already init
    if (getIsInit()) return ReturnCodes::Success;

    // otherwise init sub-components
    if (LEDController::init() != ReturnCodes::Success) {
        cerr << "Failed to properly init LEDs" << endl;
        return ReturnCodes::Error;
    }
    if (ButtonController::init() != ReturnCodes::Success) {
        cerr << "Failed to properly init buttons" << endl;
        return ReturnCodes::Error;
    }

    return ReturnCodes::Success;
}

ReturnCodes GPIO_Controller::setShouldThreadExit(const bool new_status) const {
    // only return success if both were successful
    return \
        LEDController::setShouldThreadExit(new_status) == ReturnCodes::Success &&
        ButtonController::setShouldThreadExit(new_status) == ReturnCodes::Success ?
            ReturnCodes::Success : ReturnCodes::Error;

}


ReturnCodes GPIO_Controller::run(const CLI::Results::ParseResults& flags) const {
    // get required variables from flag mapping
    const auto& mode        {flags.at(CLI::Results::MODE)};
    const auto& colors      {Helpers::splitStr(',', flags.at(CLI::Results::COLORS))};
    const auto& interval    {
                                static_cast<unsigned int>(
                                    std::stoi(flags.at(CLI::Results::INTERVAL))
                                )
                            };
    const auto& duration    {std::stoi(flags.at(CLI::Results::DURATION))};
    const auto& rate        {
                                static_cast<unsigned int>(
                                    std::stoi(flags.at(CLI::Results::RATE))
                                )
                            };

    mode_to_action.searchAndCall<void>(
        *this, // need to pass reference to this object
        mode, // key to the function to call
        // pass the actual params needed by functions
        colors,
        interval,
        duration,
        rate
    );

    return ReturnCodes::Success;
}

/********************************************* Helper Functions ********************************************/

// std::unordered_map<std::string, int> GPIO_Controller::generateLedBtnPairs() {}

Helpers::Map::ClassFnMap<GPIO_Controller> GPIO_Controller::createFnMap() const {
    Helpers::Map::ClassFnMap<GPIO_Controller> to_rtn;
    // TODO: Figure out way to make reinterpret_cast automatic
    to_rtn["Blink"]       = reinterpret_cast<void(LEDController::*)()>(&LEDController::blinkLEDs);
    to_rtn["Intensity"]   = reinterpret_cast<void(LEDController::*)()>(&LEDController::LEDIntensity);
    to_rtn["Btns"]        = reinterpret_cast<void(ButtonController::*)()>(&ButtonController::detectBtnPress);
    return to_rtn;
}


}; // end of gpio namespace