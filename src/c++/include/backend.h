#ifndef EVENT_LISTENER_H
#define EVENT_LISTENER_H

// Standard Includes
#include <iostream>
#include <string>

// Our Includes
#include "constants.h"
#include "tcp_base.h" // shared_ptr to base class (for updatePkt())

// 3rd Party Includes
#include <crow.h>

namespace RPI {

namespace UI {

/**
 * @brief Class that extends the Crow Web App API to manage all the routes
 */
class WebApp {
    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a new Event Listener object
         * 
         * @param tcp_client ptr to the tcp client
         * @param port The port to run the client at (defaults to 8080)
         */
        WebApp(const std::shared_ptr<RPI::Network::TcpBase> tcp_client, const int port=8080);
        virtual ~WebApp();

        /********************************************* Getters/Setters *********************************************/


        /********************************************* Web UI Functions *********************************************/

        /**
         * @brief Blocking function that starts up the web app
         */
        void startWebApp();

    private:
        /******************************************** Private Variables ********************************************/

        // shared pointer to the base casted TcpClient object (used to call updatePkt to trigger a send)
        std::shared_ptr<RPI::Network::TcpBase> client_ptr;
        const int web_port;                 // port the web app should use
        crow::SimpleApp web_app;            // the web app object

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Function called by constructor to help setup all the app's webpages
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes setupSites();


}; // end of WebApp class

}; // end of UI namespace

}; // end of RPI namespace

#endif
