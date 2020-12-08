#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>
#include <atomic>
#include <thread>

// Our Includes
#include "map_helpers.hpp"
#include "print_helpers.hpp"
#include "timing.hpp"
#include "constants.h"

// 3rd Party Includes
#include <wiringPi.h>
#include <softPwm.h>

namespace gpio {
namespace LED {

/**
 * @brief Handles all LED operations
 */
class LEDController {
    public:
        /********************************************** Constructors **********************************************/
        LEDController();
        virtual ~LEDController();

        /********************************************* Getters/Setters *********************************************/
        /**
         * @brief: Gets a list of LED colors
         * @returns: Vector<string> of each color
         */
        std::vector<std::string> getLedColorList();

        /**
         * @brief Set whether the thread should stop
         * @param new_status The new status (true = stop, false = keep going) 
         * @return ReturnCodes 
         */
        ReturnCodes setShouldThreadExit(const bool new_status);
        /**
         * @brief Get whether the thread should stop
         * @return std::atomic_bool 
         */
        const std::atomic_bool& getShouldThreadExit() const;

        /********************************************* LED Functions *********************************************/
        /**
         * @brief Blink specific leds at the set interval (default to 1s) 
         * @param colors The list of colors to blink
         * @param interval The interval to blink the leds at (in ms)
         * @param duration How long to run for in ms (-1 = infinite)
         */
        void blinkLEDs(std::vector<std::string> colors, unsigned int interval=1000, int duration=-1);

        /**
         * @brief Increase specific leds brightness at the set interval (default to 1s) 
         * @param colors The list of colors to blink
         * @param interval The interval to blink the leds at (in ms)
         * @param duration How long to run for in ms (-1 = infinite)
         * @param rate The rate at which the LEDs' intensity should change (i.e. 1x, 2x, 3x)
         */
        void LEDIntensity(
            const std::vector<std::string>& colors,
            const unsigned int interval=1000,
            const int duration=-1,
            const unsigned int rate=1
        );

    private:
        /******************************************** Private Variables ********************************************/
        // Map each color to a led's corresponding pin number
        const std::map<std::string, int> color_to_leds;
        // controls whether or not to stop blocking functions (i.e. blink)
        std::atomic_bool stop_thread;


        /********************************************* Helper Functions ********************************************/
        /**
         * @brief Helps intialize the leds
         * @return ReturnCodes
         */
        ReturnCodes initLEDs();


}; // end of LEDController class


}; // end of LED namespace

}; // end of gpio namespace

#endif