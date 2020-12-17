#include "EventListener.h"


namespace RPI {

namespace UI {

/********************************************** Constructors **********************************************/

EventListener::EventListener(const std::shared_ptr<RPI::Network::TcpBase> tcp_client)
    : client_ptr(tcp_client)
{
    // stub
}

EventListener::~EventListener() {
    // stub
}

/********************************************* Getters/Setters *********************************************/


/********************************************* Helper Functions ********************************************/


}; // end of UI namespace

}; // end of RPI namespace

