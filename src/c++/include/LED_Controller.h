#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <map>

// Our Includes
#include "map_helpers.hpp"
#include "constants.h"

// 3rd Party Includes
#include <wiringPi.h>

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