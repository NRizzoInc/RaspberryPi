#ifndef EVENT_LISTENER_H
#define EVENT_LISTENER_H

// Standard Includes
#include <iostream>
#include <string>

// Our Includes
#include "tcp_base.h" // shared_ptr to base class (for updatePkt())

// 3rd Party Includes
#include <gainput/gainput.h>

namespace RPI {

namespace UI {

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

        /********************************************* Helper Functions ********************************************/


}; // end of EventListener class

}; // end of UI namespace

}; // end of RPI namespace

#endif
