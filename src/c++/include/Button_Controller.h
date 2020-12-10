#ifndef BUTTON_CONTROLLER_H
#define BUTTON_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <atomic>

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
        virtual ReturnCodes init() const;

        /********************************************* Getters/Setters *********************************************/
        /**
         * @brief: Gets a list of Button colors
         * @returns: Vector<string> of each color
         */
        std::vector<std::string> getBtnColorList() const;

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

        /******************************************** Button Functions ********************************************/


    private:
        /******************************************** Private Variables ********************************************/
        // Map each color to a button's corresponding pin number
        const std::unordered_map<std::string, int> color_to_btns;
        /**
         * @brief Controls whether or not to stop blocking functions (i.e. blink)
         * @note Is mutable so that it can be modified in const functions safely
         */
        mutable std::atomic_bool stop_thread;
        mutable bool isInit;


        /********************************************* Helper Functions ********************************************/

}; // end of ButtonController class

}; // end of Button namespace


}; // end of gpio namespace

#endif