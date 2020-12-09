#ifndef GPIO_CONTROLLER_H
#define GPIO_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>

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

        /********************************************* Getters/Setters *********************************************/
        /**
         * @brief: Gets a list of colors that are shared between LEDs & Buttons
         * @returns: Vector<string> of each color
         */
        std::vector<std::string> getPairColorList() const;

        std::vector<std::string> getModes() const;

        /*********************************************** GPIO Helpers **********************************************/

    private:
        /******************************************** Private Variables ********************************************/
        const std::unordered_map<std::string, int> color_to_led_btn_pairs;
        // maps a string (mode name) to a gpio function
        // cannot bc constant due to nature of how it must be constructed
        const Helpers::Map::FnMap& mode_to_action;

        /********************************************* Helper Functions ********************************************/
        /**
         * @brief: Helps construct color_to_led_btn_pairs based on what colors they share
         * @returns: The constructed map of shared colors
         */
        // std::unordered_map<std::string, int> generateLedBtnPairs();


        /**
         * @brief Create a Fn Map object
         * @return The function map object 
         * @note Use in constructor with std::move to save in reference variable
         */
        Helpers::Map::FnMap createFnMap () const;

};


}; // end of gpio namespace

#endif
