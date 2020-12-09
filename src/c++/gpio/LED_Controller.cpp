#include "LED_Controller.h"

// to keep lines short
using std::cout;
using std::cerr;
using std::endl;
using ms = std::chrono::milliseconds;


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
    cout << "Resetting LED Pins" << endl;
    for (auto& color_pin : color_to_leds) {
        softPwmWrite(color_pin.second, Constants::LED_SOFT_PWM_MIN);
    }
    isInit = false;
}

ReturnCodes LEDController::init() {
    // if already init, stop now
    if (isInit) return ReturnCodes::Success;

    // setup pins for their purpose
    if (wiringPiSetup() == -1) {
        return ReturnCodes::Error;
    }

    for (auto& led_entry : color_to_leds) {
        // setup each led as a software PWM LED (RPI only has 2 actual pwm pins)
        softPwmCreate(led_entry.second, Constants::LED_SOFT_PWM_MIN, Constants::LED_SOFT_PWM_RANGE);
    }

    isInit = true;
    return ReturnCodes::Success;
}

/********************************************* Getters/Setters *********************************************/
std::vector<std::string> LEDController::getLedColorList() {
    return Helpers::Map::getMapKeys(color_to_leds);
}

ReturnCodes LEDController::setShouldThreadExit(const bool new_status) {
    stop_thread = new_status;
    return ReturnCodes::Success;
}

const std::atomic_bool& LEDController::getShouldThreadExit() const {
    return stop_thread;
}

bool LEDController::getIsInit() const {
    return isInit;
}

/********************************************* LED Functions *********************************************/

void LEDController::blinkLEDs(
    const std::vector<std::string>& colors,
    const unsigned int interval,
    const int duration
) {
    cout << "Blinking: " << Helpers::createVecStr(colors) << endl;
    cout << "Interval: " << interval << "ms" << endl;
    cout << "Duration: " << duration << "ms" << endl;

    // keep track of time/duration
    const auto start_time = std::chrono::steady_clock::now();

    while (
        !getShouldThreadExit() &&
        // if duration == -1 : run forever
        (duration == -1 || Helpers::hasTimeElapsed(start_time, duration, ms(1)))
    ) {
        // on
        for (auto& to_blink : colors) {
            softPwmWrite(color_to_leds.at(to_blink), Constants::LED_SOFT_PWM_MAX);
        }
        std::this_thread::sleep_for(ms(interval));

        // off
        for (auto& to_blink : colors) {
            softPwmWrite(color_to_leds.at(to_blink), Constants::LED_SOFT_PWM_MIN);
        }
        std::this_thread::sleep_for(ms(interval));
    }
}

void LEDController::LEDIntensity(
    const std::vector<std::string>& colors,
    const unsigned int interval,
    const int duration,
    const unsigned int rate
) {
    cout << "Changing intensity for: " << Helpers::createVecStr(colors) << endl;
    cout << "Interval: " << interval << "ms" << endl;
    cout << "Duration: " << duration << "ms" << endl;
    cout << "Change Rate: " << rate << 'x' << endl;

    // keep track of time/duration
    const auto start_time = std::chrono::steady_clock::now();

    const unsigned int num_intervals {Constants::LED_SOFT_PWM_RANGE / rate};
    unsigned int curr_interval_count {0};
    while (
        !getShouldThreadExit() &&
        // if duration == -1 : run forever
        (duration == -1 || Helpers::hasTimeElapsed(start_time, duration, ms(1)))
    ) {
        // get current intensity by finding brightness as current interval count
        const float perc_interv {
            static_cast<float>(curr_interval_count) / static_cast<float>(num_intervals)
        };
        const unsigned int curr_brightness {
            static_cast<unsigned int>(perc_interv * Constants::LED_SOFT_PWM_RANGE)
        };
        curr_interval_count = (curr_interval_count+1) % (num_intervals+1); // +1 to reach max

        // change LEDs brightness
        for (auto& to_change : colors) {
            softPwmWrite(color_to_leds.at(to_change), curr_brightness);
        }
        std::this_thread::sleep_for(ms(interval));
    }
}

/********************************************* Helper Functions ********************************************/


}; // end of LED namespace

}; // end of gpio namespace
