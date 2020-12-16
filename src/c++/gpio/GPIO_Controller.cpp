#include "GPIO_Controller.h"

using std::cout;
using std::cerr;
using std::endl;

namespace RPI {

namespace gpio {

/********************************************** Constructors **********************************************/
GPIO_Controller::GPIO_Controller()
    // call constructors for parents
    : LED::LEDController()
    , Button::ButtonController()

    // init vars
    , color_to_led_btn_pairs (generateLedBtnPairs())
    , mode_to_action (createFnMap())
    , run_thread{}
    , started_thread{false}
    , has_cleaned_up{false}
{
    // stub
}

GPIO_Controller::~GPIO_Controller() {
    // stub
}

ReturnCodes GPIO_Controller::cleanup() {
    // dont double cleanup
    if (has_cleaned_up) return ReturnCodes::Success;

    // wait to block until a thread has been setup
    // otherwise thread is empty and joins immediately
    // but max out time to wait or else program gets blocked here if thread is never started
    std::unique_lock<std::mutex> lk{thread_mutex};
    thread_cv.wait_for(
        lk,
        std::chrono::milliseconds(200),
        [&](){ return started_thread.load(); }
    );

    // block until thread ends
    if (run_thread.joinable()) {
        run_thread.join();
    }

    has_cleaned_up = true;
    return ReturnCodes::Success;
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

    // set callback so that when the button is pressed, the LED's state changes
    ButtonController::setBtnCallback([&](const std::string& color, const bool btn_state){
        if(setLED(color, btn_state) != ReturnCodes::Success) {
            cerr << "Failed to set LED " << color << " to " << btn_state << endl;
        }
    });

    return ReturnCodes::Success;
}

ReturnCodes GPIO_Controller::setShouldThreadExit(const bool new_status) const {
    // only return success if both were successful
    return \
        LEDController::setShouldThreadExit(new_status) == ReturnCodes::Success &&
        ButtonController::setShouldThreadExit(new_status) == ReturnCodes::Success ?
            ReturnCodes::Success : ReturnCodes::Error;

}

ReturnCodes GPIO_Controller::gpioHandlePkt(const Network::CommonPkt& pkt) const {
    const auto& leds_status {pkt.cntrl.led};
    setLED("blue",          leds_status.blue);
    setLED("green",         leds_status.green);
    setLED("red",           leds_status.red);
    setLED("yellow",        leds_status.yellow);
    return ReturnCodes::Success;
}


ReturnCodes GPIO_Controller::run(const CLI::Results::ParseResults& flags) {
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

    // start up selected function based on mode in a thread (joined in destructor)
    run_thread = std::thread{
        std::bind(&GPIO_Controller::callSelFn, this,
            // the actual arguments needed by the function
            mode,
            colors,
            interval,
            duration,
            rate
        )
    };

    return ReturnCodes::Success;
}

/********************************************* Helper Functions ********************************************/

MapParentMaps GPIO_Controller::generateLedBtnPairs() const {
    MapParentMaps to_rtn;

    // compile list of colors both LED & Buttons share
    auto& led_mapping {getLedMap()};
    auto& btn_mapping {getBtnMap()};

    // only add to master list if both contain the string
    for (std::string color : Helpers::Map::getMapKeys(led_mapping)) {
        if (btn_mapping.find(color) != btn_mapping.end()) {
            const auto to_insert = std::make_pair(
                led_mapping.at(color),
                btn_mapping.at(color)
            );
            // breaks with .insert()
            to_rtn.emplace(color, to_insert);
        }
    }

    return to_rtn;
}

void GPIO_Controller::doNothing() const {
    // sometimes you just gotta be a bit sassy
    // cout << "You chose the option to do nothing... you should rethink your life choices" << endl;
}

void GPIO_Controller::callSelFn(
    const std::string& mode,
    const std::vector<std::string>& colors,
    const unsigned int& interval,
    const int& duration,
    const unsigned int& rate
) const {
    mode_to_action.searchAndCall<void>(
        *this, // need to pass reference to this object
        mode, // key to the function to call
        // pass the actual params needed by functions
        colors,
        interval,
        duration,
        rate
    );
}


Helpers::Map::ClassFnMap<GPIO_Controller> GPIO_Controller::createFnMap() const {
    Helpers::Map::ClassFnMap<GPIO_Controller> to_rtn;
    // TODO: Figure out way to make reinterpret_cast automatic
    to_rtn["blink"]       = reinterpret_cast<void(LEDController::*)()>(&LEDController::blinkLEDs);
    to_rtn["intensity"]   = reinterpret_cast<void(LEDController::*)()>(&LEDController::LEDIntensity);
    to_rtn["btns"]        = reinterpret_cast<void(ButtonController::*)()>(&ButtonController::detectBtnPress);
    to_rtn["server"]      = reinterpret_cast<void(GPIO_Controller::*)()>(&GPIO_Controller::doNothing);
    to_rtn["client"]      = reinterpret_cast<void(GPIO_Controller::*)()>(&GPIO_Controller::doNothing);
    to_rtn["none"]        = reinterpret_cast<void(GPIO_Controller::*)()>(&GPIO_Controller::doNothing);
    return to_rtn;
}


}; // end of gpio namespace

}; // end of RPI namespace
