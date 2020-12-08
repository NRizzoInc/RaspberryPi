#ifndef GPIO_CONTROLLER_H
#define GPIO_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <map>

// Our Includes
#include "map_helpers.hpp"
#include "constants.h"
#include "LED_Controller.h"
#include "Button_Controller.h"

// 3rd Party Includes
#include <wiringPi.h>

namespace gpio {

/**
 * @brief Handles all GPIO related operations
 */
class GPIO_Controller : public LED::LEDController, public Button::ButtonController {
    public:
        /********************************************** Constructors **********************************************/
        GPIO_Controller();
        virtual ~GPIO_Controller();

        /********************************************* Public Helpers *********************************************/
        /**
         * @brief: Gets a list of colors that are shared between LEDs & Buttons
         * @returns: Vector<string> of each color
         */
        std::vector<std::string> getPairColorList();

        /**
         * @brief Helps intialize all gpio components
         * @return ReturnCodes
         */
        ReturnCodes initGPIOs();

    private:
        /******************************************** Private Variables ********************************************/
        const std::map<std::string, int> color_to_led_btn_pairs;

        /********************************************* Helper Functions ********************************************/
        /**
         * @brief: Helps construct color_to_led_btn_pairs based on what colors they share
         * @returns: The constructed map of shared colors
         */
        // std::map<std::string, int> generateLedBtnPairs();

};


}; // end of gpio namespace

#endif
