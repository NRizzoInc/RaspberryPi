#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <thread>
#include <chrono>
#include <atomic>
#include <functional>

// Our Includes
#include "map_helpers.hpp"
#include "constants.h"
#include "string_helpers.hpp"
#include "timing.hpp"

// 3rd Party Includes
#include <wiringPi.h>


namespace gpio {
namespace Button {

using BtnMapVal = std::pair<const int, bool>;
using BtnMap = std::unordered_map<std::string, BtnMapVal>;

// the type of the callback used when a button's state has changed
using BtnCallback = std::function<void(const std::string color, const bool btn_state)>;

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
        virtual ReturnCodes init() const;

        /********************************************* Getters/Setters *********************************************/
        /**
         * @brief: Gets a list of Button colors
         * @returns: Vector<string> of each color
         */
        std::vector<std::string> getBtnColorList() const;

        const BtnMap& getBtnMap() const;

        virtual bool getIsInit() const;
        ReturnCodes setIsInit(const bool new_state) const;

        /**
         * @brief Set whether the thread should stop
         * @param new_status The new status (true = stop, false = keep going) 
         * @return ReturnCodes
         * @note can be const because underlying bool is mutable
         */
        virtual ReturnCodes setShouldThreadExit(const bool new_status) const;

        /**
         * @brief Get whether the thread should stop
         * @return std::atomic_bool 
         */
        virtual const std::atomic_bool& getShouldThreadExit() const;

        /**
         * @brief Set the callback to occur when a button's state is changed
         * (i.e. pressed => released or released => pressed)
         * @arg Callback of format: std::function<void(const std::string color, const bool btn_state)>
         * @arg Callback's "color" is string representing the color of the changed button
         * @arg Callback's "btn_state" represents the button's new state (false = released, true = pressed)
         * @return Callback should return void
         * @return ReturnCodes Was setting it successful 
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
        mutable BtnMap color_to_btns;

        /**
         * @brief Controls whether or not to stop blocking functions (i.e. blink)
         * @note Is mutable so that it can be modified in const functions safely
         */
        mutable std::atomic_bool stop_thread;
        mutable bool isInit;

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

#endif