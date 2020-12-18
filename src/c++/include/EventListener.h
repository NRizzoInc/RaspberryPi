#ifndef EVENT_LISTENER_H
#define EVENT_LISTENER_H

// Standard Includes
#include <iostream>
#include <string>

// Our Includes
#include "constants.h"
#include "tcp_base.h" // shared_ptr to base class (for updatePkt())

// 3rd Party Includes
#include <gainput/gainput.h>
#include <crow.h>

namespace RPI {

namespace UI {

enum class ButtonCodes : int {
    ButtonConfirm
}; // end of Button enum class

/**
 * @brief Class that extends the gainput library for UI event listeners
 * @note Following usage guide: https://github.com/jkuhlmann/gainput/tree/v1.0.0#usage
 */
class EventListener {
    public:
        /********************************************** Constructors **********************************************/
        EventListener(const std::shared_ptr<RPI::Network::TcpBase> tcp_client);
        virtual ~EventListener();

        /********************************************* Getters/Setters *********************************************/


        /********************************************* Event Functions *********************************************/


    private:
        /******************************************** Private Variables ********************************************/

        // shared pointer to the base casted TcpClient object (used to call updatePkt to trigger a send)
        std::shared_ptr<RPI::Network::TcpBase> client_ptr;

        /******************************************** Gainput Variables ********************************************/

        // manages all devices
        gainput::InputManager manager;

        // stores info about the keyboard
        const gainput::DeviceId keyboardId;

        // maps device ids to buttons
        gainput::InputMap input_dev_map;

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Helps set up the object
         * @note Should be called in constructor
         * @return ReturnCodes: Error if there is any problem, Success otherwise
         */
        ReturnCodes setupListener();

}; // end of EventListener class

}; // end of UI namespace

}; // end of RPI namespace

#endif
