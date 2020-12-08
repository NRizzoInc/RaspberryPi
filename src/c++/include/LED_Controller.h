#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <chrono>

// Our Includes
#include "map_helpers.hpp"
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

        /********************************************* Public Helpers *********************************************/
        /**
         * @brief: Gets a list of LED colors
         * @returns: Vector<string> of each color
         */
        std::vector<std::string> getLedColorList();

        /**
         * @brief Blink specific leds at the set interval (default to 1s) 
         * @param colors The list of colors to blink
         * @param interval The interval to blink the leds at (in ms)
         * @param duration How long to run for in ms (-1 = infinite)
         */
        void blinkLEDs(std::vector<std::string> colors, unsigned int interval=1000, int duration=-1);

    private:
        /******************************************** Private Variables ********************************************/
        // Map each color to a led's corresponding pin number
        const std::map<std::string, int> color_to_leds;


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