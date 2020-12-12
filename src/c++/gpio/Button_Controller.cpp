#include "Button_Controller.h"

using std::cout;
using std::cerr;
using std::endl;


namespace gpio {
namespace Button {

/********************************************** Constructors **********************************************/
ButtonController::ButtonController()
    : color_to_btns ({
            // get mappings in terminal with `gpio readall`
            // go by "wPi" column
            // init each button as "unpressed" = false
            {"red",     std::make_pair(8, false)},
            {"yellow",  std::make_pair(9, false)},
            {"green",   std::make_pair(7, false)},
            {"blue",    std::make_pair(0, false)}
        })
    , stop_thread(false)
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

    // setup each pin corresponding to a color to be an input button
    for (auto& btn_entry : color_to_btns) {
        // set pins as inputs
        // https://github.com/WiringPi/WiringPi/blob/master/examples/Gertboard/buttons.c
        const int pin_num {btn_entry.second.first};
        pinMode(pin_num, INPUT);
        pullUpDnControl(pin_num, PUD_UP);
    }

    setIsInit(true);
    return ReturnCodes::Success;
}

/********************************************* Getters/Setters *********************************************/
std::vector<std::string> ButtonController::getBtnColorList() const {
    return Helpers::Map::getMapKeys(color_to_btns);
}

const BtnMap& ButtonController::getBtnMap() const {
    return color_to_btns;
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
void ButtonController::detectBtnPress(
    const std::vector<std::string>& colors,
    __attribute__((unused)) const unsigned int& interval,
    const int& duration,
    __attribute__((unused)) const unsigned int& rate
) const {
    cout << "Watching Buttons: " << Helpers::createVecStr(colors) << endl;
    cout << "Duration: " << duration << "ms" << endl;

    // keep track of time/duration
    const auto start_time = std::chrono::steady_clock::now();

    while (
        !ButtonController::getShouldThreadExit() &&
        // if duration == -1 : run forever
        (duration == -1 || Helpers::Timing::hasTimeElapsed(start_time, duration, std::chrono::milliseconds(1)))
    ) {
        for (auto& btn_color : colors) {
            // get last state and compare with current state
            // btn_status is not const incase need to change its pressed state
            auto&           btn_status      {color_to_btns.at(btn_color)};
            const int&      pin_num         {btn_status.first};
            const bool&     last_state      {btn_status.second};
            const bool&     is_pressed      {isDepressed(pin_num)};
            const bool&     state_changed   {last_state != is_pressed};

            // only change state value in storage if there was a change (reduce number of writes)
            if (state_changed) {
                // update status in regiser
                color_to_btns[btn_color].second = is_pressed;
                
                if (is_pressed) {
                    cout << "Pressed: " << btn_color << endl;
                } else {
                    cout << "Released: " << btn_color << endl;
                }
            }
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
}


/********************************************* Helper Functions ********************************************/

bool ButtonController::isDepressed(const int pin) const {
    // note: inputs read opposite of what you would think
    // inputs read HIGH(1) when not pressed
    return digitalRead(pin) == LOW;
}


}; // end of Button namespace

}; // end of gpio namespace
