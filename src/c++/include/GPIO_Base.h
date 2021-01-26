#ifndef GPIO_BASE_H
#define GPIO_BASE_H

// Standard Includes
#include <iostream>
#include <fstream> // needed to open file to check if valid pi
#include <string>
#include <atomic>
#include <optional>

// Our Includes
#include "constants.h"
#include "string_helpers.hpp"

// 3rd Party Includes
#include <wiringPi.h>

namespace RPI {
namespace gpio {

/**
 * @brief Base class for all gpio classes
 * (Implements some common functions to not repeat code)
 * 
 */
class GPIOBase {

    public:
        /********************************************** Constructors **********************************************/
        GPIOBase();
        virtual ~GPIOBase();

        virtual ReturnCodes init() const;

        /********************************************* Getters/Setters *********************************************/

        virtual bool getIsInit() const;
        virtual ReturnCodes setIsInit(const bool new_state) const;

        /**
         * @brief Set whether the thread should stop
         * @param new_status The new status (true = stop, false = keep going) 
         * @return ReturnCodes
         * @note can be const because underlying bool is mutable
         */
        virtual ReturnCodes setShouldThreadExit(const bool new_status) const;

        /**
         * @brief Get whether the thread should stop
         * @return True if thread should exit
         */
        virtual bool getShouldThreadExit() const;

        /****************************************** Basic Board Functions ******************************************/

        /**
         * @brief Determines if the device running this code is compatible with this RPI code
         * (Enables programs to selectively not enable gpio code if running on non-rpi)
         * @return true Is a valid rpi that can run the gpio code
         * @return false Is an invalid rpi (aka not a pi) and it shouldnt try to init gpio classes
         */
        virtual bool isValidRPI() const;

    private:
        mutable bool isInit;
        /**
         * @brief Controls whether or not to stop blocking functions (i.e. blink)
         * @note Is mutable so that it can be modified in const functions safely
         */
        mutable std::atomic_bool stop_thread;
        static std::optional<bool> is_valid_pi; // for all derived classes, should only check validity once

}; // end of GPIOBase


}; // end of gpio namespace

}; // end of RPI namespace

#endif
