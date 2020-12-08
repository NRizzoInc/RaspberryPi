#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

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
namespace Button {

/**
 * @brief Handles all button operations
 */
class ButtonController {
    public:
        /********************************************** Constructors **********************************************/
        ButtonController();
        virtual ~ButtonController();

        /********************************************* Public Helpers *********************************************/
        /**
         * @brief: Gets a list of Button colors
         * @returns: Vector<string> of each color
         */
        std::vector<std::string> getBtnColorList();

    private:
        /******************************************** Private Variables ********************************************/
        // Map each color to a button's corresponding pin number
        const std::map<std::string, int> color_to_btns;


        /********************************************* Helper Functions ********************************************/
        /**
         * @brief Helps intialize the buttons
         * @return ReturnCodes
         */
        ReturnCodes initButtons();

}; // end of ButtonController class

}; // end of Button namespace


}; // end of gpio namespace

#endif