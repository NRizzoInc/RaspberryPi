#include "EventListener.h"


namespace RPI {

namespace UI {

using std::cout;
using std::cerr;
using std::endl;

/********************************************** Constructors **********************************************/

EventListener::EventListener(const std::shared_ptr<RPI::Network::TcpBase> tcp_client, const int port)
    : client_ptr{tcp_client}
    , web_port{port}
    , web_app{}
{
    setupSites();
}

EventListener::~EventListener() {
    // stub
}

/********************************************* Getters/Setters *********************************************/


void EventListener::startWebApp() {
    // start running the web app
    web_app.port(web_port).run();
}

/********************************************* Helper Functions ********************************************/

ReturnCodes EventListener::setupSites() {
    web_app.route_dynamic(Constants::UI::URL_MAIN)
    ([]() {
        return "Hello world!";
    });

    return ReturnCodes::Success;
}


}; // end of UI namespace

}; // end of RPI namespace

