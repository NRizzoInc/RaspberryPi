#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

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

        /**
         * @brief Helps intialize the buttons
         * @return ReturnCodes
         */
        virtual ReturnCodes init();

        /********************************************* Getters/Setters *********************************************/
        /**
         * @brief: Gets a list of Button colors
         * @returns: Vector<string> of each color
         */
        std::vector<std::string> getBtnColorList();

        bool getIsInit() const;

        /******************************************** Button Functions ********************************************/


    private:
        /******************************************** Private Variables ********************************************/
        // Map each color to a button's corresponding pin number
        const std::unordered_map<std::string, int> color_to_btns;
        bool isInit;


        /********************************************* Helper Functions ********************************************/

}; // end of ButtonController class

}; // end of Button namespace


}; // end of gpio namespace

#endif