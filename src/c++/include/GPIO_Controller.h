#ifndef GPIO_CONTROLLER_H
#define GPIO_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <map>

// Our Includes
#include "map_helpers.hpp"

// 3rd Party Includes
#include <wiringPi.h>

namespace gpio {

class GPIO_Controller {
    public:
        /********************************************** Constructors **********************************************/
        GPIO_Controller();
        virtual ~GPIO_Controller();

        /********************************************* Public Helpers *********************************************/
        /**
         * @Brief: Gets a list of LED colors
         * @Returns: Vector<string> of each color
         */
        std::vector<std::string> getLedColorList();
        /**
         * @Brief: Gets a list of Button colors
         * @Returns: Vector<string> of each color
         */
        std::vector<std::string> getBtnColorList();
        /**
         * @Brief: Gets a list of colors that are shared between LEDs & Buttons
         * @Returns: Vector<string> of each color
         */
        std::vector<std::string> getPairColorList();

    private:
        /******************************************** Private Variables ********************************************/
        // Map each color to a led's corresponding pin number
        const std::map<std::string, int> color_to_leds;
        // Map each color to a button's corresponding pin number
        const std::map<std::string, int> color_to_btns;
        const std::map<std::string, int> color_to_led_btn_pairs;

        /********************************************* Helper Functions ********************************************/
        /**
         * @Brief: Helps construct color_to_led_btn_pairs based on what colors they share
         * @Returns: The constructed map of shared colors
         */
        // std::map<std::string, int> generateLedBtnPairs();

};


}; // end of gpio namespace

#endif
