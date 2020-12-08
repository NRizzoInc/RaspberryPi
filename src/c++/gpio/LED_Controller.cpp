#include "LED_Controller.h"

using std::cout;
using std::cerr;
using std::endl;


namespace gpio {
namespace LED {

/********************************************** Constructors **********************************************/
LEDController::LEDController()
    // pin mappings -- http://wiringpi.com/pins/
    // gpio readall -- care about WPi column
    : color_to_leds ({
            {"red",     5}, // gpio5 /pin(BCM) 24 (can use variable brightness)
            {"yellow",  4},
            {"green",   1},
            {"blue",    16}
        })
    , stop_thread(false)
{
    // init LEDs on board
    if(initLEDs() != ReturnCodes::Success) {
        cerr << "Failed to properly init LEDs" << endl;
    }
}

LEDController::~LEDController() {
    // set all LEDs to off at end
    cout << "calling LED destructor" << endl;
    for (auto& color_pin : color_to_leds) {
        softPwmWrite(color_pin.second, Constants::LED_SOFT_PWM_MIN);
    }
}

/********************************************* Public Helpers *********************************************/
std::vector<std::string> LEDController::getLedColorList() {
    return Helpers::getMapKeys(color_to_leds);
}

void LEDController::blinkLEDs(std::vector<std::string> colors, unsigned int interval, int duration) {
    // keep track of time/duration
    const auto start_time = std::chrono::steady_clock::now();

    // to keep lines short, use a lambda to keep track of end time
    // & temporarily shorten syntax for ms
    using ms = std::chrono::milliseconds;
    const auto hasTimeElapsed = [&](){
        const auto end_time = std::chrono::steady_clock::now();
        const auto elapsed_time = std::chrono::duration_cast<ms>(end_time - start_time).count();
        return elapsed_time > duration;
    };

    // if duration == -1 : run forever
    while (!getShouldThreadExit() && (duration == -1 || hasTimeElapsed())) {
        // on
        for (auto& to_blink : colors) {
            softPwmWrite(color_to_leds.at(to_blink), Constants::LED_SOFT_PWM_MAX);
        }
        delay(interval);

        // off
        for (auto& to_blink : colors) {
            softPwmWrite(color_to_leds.at(to_blink), Constants::LED_SOFT_PWM_MIN);
        }
        delay(interval);
    }
}


ReturnCodes LEDController::setShouldThreadExit(const bool new_status) {
    stop_thread = new_status;
    return ReturnCodes::Success;
}

const std::atomic_bool& LEDController::getShouldThreadExit() const {
    return stop_thread;
}

/********************************************* Helper Functions ********************************************/
ReturnCodes LEDController::initLEDs() {
    // setup pins for their purpose
    if (wiringPiSetup() == -1) {
        return ReturnCodes::Error;
    }

    for (auto& led_entry : color_to_leds) {
        // setup each led as a software PWM LED (RPI only has 2 actual pwm pins)
        softPwmCreate(led_entry.second, Constants::LED_SOFT_PWM_MIN, Constants::LED_SOFT_PWM_MAX);
    }
    return ReturnCodes::Success;
}


}; // end of LED namespace

}; // end of gpio namespace
