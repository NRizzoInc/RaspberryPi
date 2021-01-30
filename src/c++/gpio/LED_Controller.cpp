#include "LED_Controller.h"

// to keep lines short
using std::cout;
using std::cerr;
using std::endl;


namespace RPI {
namespace gpio {
namespace LED {

/**************************************** Static Member Variables *****************************************/

const LEDMap LEDController::color_to_leds {
    {"red",     22},
    {"yellow",  23},
    {"green",   24},
    {"blue",    25}
};


/********************************************** Constructors **********************************************/
LEDController::LEDController(const bool verbosity)
    // pin mappings -- http://wiringpi.com/pins/
    // gpio readall -- care about WPi column
    : GPIOBase{verbosity}
{
    // stub
}

LEDController::~LEDController() {
    // set all LEDs to off at end
    if (getIsInit()) {
        cout << "Resetting LED Pins" << endl;
        for (auto& color_pin : color_to_leds) {
            // set off and stop gpio pin
            softPwmWrite(color_pin.second, Constants::GPIO::LED_SOFT_PWM_MIN);
            softPwmStop(color_pin.second);
        }
        setIsInit(false);
    }
}

ReturnCodes LEDController::init() const {
    // if already init, stop now
    if (getIsInit()) return ReturnCodes::Success;

    if(GPIOBase::init() != ReturnCodes::Success) {
        return ReturnCodes::Error;
    }

    for (auto& led_entry : color_to_leds) {
        // setup each led as a software PWM LED (RPI only has 2 actual pwm pins)
        softPwmCreate(
            led_entry.second,
            Constants::GPIO::LED_SOFT_PWM_MIN,
            Constants::GPIO::LED_SOFT_PWM_RANGE
        );
    }

    setIsInit(true);
    return ReturnCodes::Success;
}

/********************************************* Getters/Setters *********************************************/
std::vector<std::string> LEDController::getLedColorList() {
    return Helpers::Map::getMapKeys(color_to_leds);
}

const LEDMap& LEDController::getLedMap() {
    return color_to_leds;
}

/********************************************* LED Functions *********************************************/

ReturnCodes LEDController::setLED(const std::string& led_color, const bool new_state) const {
    // call overloaded version which accepts the pin number
    return setLED(color_to_leds.at(led_color), new_state);
}

ReturnCodes LEDController::setLED(const int pin_num, const bool new_state) const {
    // true = on, false = off
    const int on_off_val {new_state ? Constants::GPIO::LED_SOFT_PWM_MAX : Constants::GPIO::LED_SOFT_PWM_MIN};
    softPwmWrite(pin_num, on_off_val);
    return ReturnCodes::Success;
}


void LEDController::blinkLEDs(
    const std::vector<std::string>& colors,
    const unsigned int& interval,
    const int& duration,
    __attribute__((unused)) const unsigned int& rate
) const {
    cout << "Blinking: " << Helpers::createVecStr(colors) << endl;
    cout << "Interval: " << interval << "ms" << endl;
    cout << "Duration: " << duration << "ms" << endl;

    // keep track of time/duration
    const auto start_time = std::chrono::steady_clock::now();

    while (
        !LEDController::getShouldThreadExit() &&
        // if duration == -1 : run forever
        (duration == -1 || Helpers::Timing::hasTimeElapsed(start_time, std::chrono::milliseconds(duration)))
    ) {
        // on
        for (auto& to_blink : colors) {
            setLED(to_blink, true);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

        // off
        for (auto& to_blink : colors) {
            setLED(to_blink, false);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));
    }
}

void LEDController::LEDIntensity(
    const std::vector<std::string>& colors,
    const unsigned int& interval,
    const int& duration,
    const unsigned int& rate
) const {
    cout << "Changing intensity for: " << Helpers::createVecStr(colors) << endl;
    cout << "Interval: " << interval << "ms" << endl;
    cout << "Duration: " << duration << "ms" << endl;
    cout << "Change Rate: " << rate << 'x' << endl;

    // keep track of time/duration
    const auto start_time = std::chrono::steady_clock::now();

    // how much time to sleep for before changing the brightness (in ms)
    // i.e.: if range of brightnesses = 100, interval = 1000ms & rate = 10x, should cycle every 1ms
    const unsigned int cycles_per_change = {static_cast<unsigned int>(Constants::GPIO::LED_SOFT_PWM_RANGE) * rate};
    const unsigned int time_between_change {interval / cycles_per_change};
    unsigned int curr_brightness {Constants::GPIO::LED_SOFT_PWM_MIN};
    while (
        !LEDController::getShouldThreadExit() &&
        // if duration == -1 : run forever
        (duration == -1 || Helpers::Timing::hasTimeElapsed(start_time, std::chrono::milliseconds(duration)))
    ) {
        curr_brightness = (curr_brightness+1) % (Constants::GPIO::LED_SOFT_PWM_MAX+1); // +1 to reach max

        // change LEDs brightness
        for (auto& to_change : colors) {
            softPwmWrite(color_to_leds.at(to_change), curr_brightness);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(time_between_change));
    }
}

/********************************************* Helper Functions ********************************************/


}; // end of LED namespace

}; // end of gpio namespace

}; // end of RPI namespace
