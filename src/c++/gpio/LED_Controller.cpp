#include "LED_Controller.h"

// to keep lines short
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
    , isInit(false)
{
    // stub
}

LEDController::~LEDController() {
    // set all LEDs to off at end
    if (getIsInit()) {
        cout << "Resetting LED Pins" << endl;
        for (auto& color_pin : color_to_leds) {
            // set off and stop gpio pin
            softPwmWrite(color_pin.second, Constants::LED_SOFT_PWM_MIN);
            softPwmStop(color_pin.second);
        }
        setIsInit(false);
    }
}

ReturnCodes LEDController::init() const {
    // if already init, stop now
    if (getIsInit()) return ReturnCodes::Success;

    // setup pins for their purpose
    if (wiringPiSetup() == -1) {
        return ReturnCodes::Error;
    }

    for (auto& led_entry : color_to_leds) {
        // setup each led as a software PWM LED (RPI only has 2 actual pwm pins)
        softPwmCreate(led_entry.second, Constants::LED_SOFT_PWM_MIN, Constants::LED_SOFT_PWM_RANGE);
    }

    setIsInit(true);
    return ReturnCodes::Success;
}

/********************************************* Getters/Setters *********************************************/
std::vector<std::string> LEDController::getLedColorList() const {
    return Helpers::Map::getMapKeys(color_to_leds);
}

ReturnCodes LEDController::setShouldThreadExit(const bool new_status) const {
    stop_thread = new_status;
    return ReturnCodes::Success;
}

const std::atomic_bool& LEDController::getShouldThreadExit() const {
    return stop_thread;
}

bool LEDController::getIsInit() const {
    return isInit;
}

ReturnCodes LEDController::setIsInit(const bool new_state) const {
    isInit = new_state;
    return ReturnCodes::Success;
}


/********************************************* LED Functions *********************************************/

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
        (duration == -1 || Helpers::Timing::hasTimeElapsed(start_time, duration, std::chrono::milliseconds(1)))
    ) {
        // on
        for (auto& to_blink : colors) {
            softPwmWrite(color_to_leds.at(to_blink), Constants::LED_SOFT_PWM_MAX);
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(interval));

        // off
        for (auto& to_blink : colors) {
            softPwmWrite(color_to_leds.at(to_blink), Constants::LED_SOFT_PWM_MIN);
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
    const unsigned int cycles_per_change = {static_cast<unsigned int>(Constants::LED_SOFT_PWM_RANGE) * rate};
    const unsigned int time_between_change {interval / cycles_per_change};
    unsigned int curr_brightness {Constants::LED_SOFT_PWM_MIN};
    while (
        !LEDController::getShouldThreadExit() &&
        // if duration == -1 : run forever
        (duration == -1 || Helpers::Timing::hasTimeElapsed(start_time, duration, std::chrono::milliseconds(1)))
    ) {
        curr_brightness = (curr_brightness+1) % (Constants::LED_SOFT_PWM_MAX+1); // +1 to reach max

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
