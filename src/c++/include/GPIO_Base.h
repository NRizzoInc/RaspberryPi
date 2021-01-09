#ifndef GPIO_BASE_H
#define GPIO_BASE_H

// Standard Includes
#include <iostream>
#include <string>
#include <atomic>

// Our Includes
#include "constants.h"

// 3rd Party Includes

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

    private:
        mutable bool isInit;
        /**
         * @brief Controls whether or not to stop blocking functions (i.e. blink)
         * @note Is mutable so that it can be modified in const functions safely
         */
        mutable std::atomic_bool stop_thread;


}; // end of GPIOBase


}; // end of gpio namespace

}; // end of RPI namespace

#endif
