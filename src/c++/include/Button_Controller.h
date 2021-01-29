#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <functional>

// Our Includes
#include "map_helpers.hpp"
#include "constants.h"
#include "string_helpers.hpp"
#include "timing.hpp"
#include "GPIO_Base.h"

// 3rd Party Includes
#include <wiringPi.h>


namespace RPI {
namespace gpio {
namespace Button {

using BtnMapVal = std::pair<const int, bool>;
using BtnMap = std::unordered_map<std::string, BtnMapVal>;

// the type of the callback used when a button's state has changed
using BtnCallback = std::function<void(const std::string& color, const bool btn_state)>;

/**
 * @brief Handles all button operations
 */
class ButtonController : public GPIOBase {
    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief LedController object that manages the RPI's Buttons.
         * @param verbosity If true, will print more information that is strictly necessary
         */
        ButtonController(const bool verbosity=false);
        virtual ~ButtonController();

        /**
         * @brief Helps intialize the buttons
         * @return ReturnCodes
         */
        virtual ReturnCodes init() const override;

        /********************************************* Getters/Setters *********************************************/
        /**
         * @brief: Gets a list of Button colors
         * @returns: Vector<string> of each color
         */
        std::vector<std::string> getBtnColorList() const;

        /**
         * @brief Get the color to button mapping
         * @note static so that it can be used to create the static member variable
         * @return BtnMap& 
         */
        static const BtnMap& getBtnMap();

        /**
         * @brief Set the callback to occur when a button's state is changed 
         * (i.e. pressed => released or released => pressed)
         * @param callback Format: std::function<void(const std::string color, const bool btn_state)>
         * @param Callback's "color" is string representing the color of the changed button
         * @param Callback's "btn_state" represents the button's new state (false = released, true = pressed)
         * @returns Callback's return is void
         */
        ReturnCodes setBtnCallback(const BtnCallback& callback) const;

        /******************************************** Button Functions ********************************************/
        
        /**
         * @brief Scans all buttons specified to see if they are pressed
         * @param colors The colors/buttons to watch for 
         * @param duration How long to run for in ms (-1 = infinite)
         */
        void detectBtnPress(
            const std::vector<std::string>& colors,
            __attribute__((unused)) const unsigned int& interval=1000,
            const int& duration=-1,
            __attribute__((unused)) const unsigned int& rate=1
        ) const;

    private:
        /******************************************** Private Variables ********************************************/
        // maps color to a buttons info: {"color": {pin#, isPressed}}
        // cannot be const because the bool "isPressed" needs to be able to change
        static BtnMap color_to_btns;

        // callback for when a button's state changes
        mutable BtnCallback btn_cb;


        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Determines if the button is being pushed
         * @param pin The pin number corresponding to the button (use color_to_btns to get)
         * @return true If is pushed
         * @return false If is not pushed
         */
        bool isDepressed(const int pin) const;

}; // end of ButtonController class

}; // end of Button namespace


}; // end of gpio namespace

}; // end of RPI namespace

#endif