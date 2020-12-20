#ifndef RPI_BACKEND_H
#define RPI_BACKEND_H

// Standard Includes
#include <iostream>
#include <string>

// Our Includes
#include "constants.h"
#include "tcp_base.h" // shared_ptr to base class (for updatePkt())
#include "web_handlers.h"

// 3rd Party Includes
#include <json.hpp>
#include "pistache/endpoint.h"

namespace RPI {

namespace UI {

// TODO: convert to variable that can be changed with input
constexpr char URL_BASE[] {"http://127.0.0.1"};

// used by WebAppUrls as keys to select specific urls
enum class WebAppUrlsNames {
    MAIN_PAGE,
    LANDING_PAGE
};

// contains actual urls as values
const std::unordered_map<WebAppUrlsNames, std::string> WebAppUrls {
    {WebAppUrlsNames::LANDING_PAGE, "/"},
    {WebAppUrlsNames::MAIN_PAGE, "/RPI-Client"}
};

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
         * @param port The port to run the client at (defaults to 5001)
         */
        WebApp(const std::shared_ptr<RPI::Network::TcpBase> tcp_client, const int port=5001);
        virtual ~WebApp();

        /********************************************* Getters/Setters *********************************************/


        /********************************************* Web UI Functions *********************************************/

        /**
         * @brief Blocking function that starts up the web app
         * @param print_urls True if should print out created available web app urls (default=true)
         */
        void startWebApp(const bool print_urls=true);

        /**
         * @brief Responsible for stopping the web app (if started it will run forever)
         */
        void stopWebApp();

    private:
        /******************************************** Private Variables ********************************************/

        // shared pointer to the base casted TcpClient object (used to call updatePkt to trigger a send)
        std::shared_ptr<RPI::Network::TcpBase> client_ptr;
        const int                   web_port;           // port the web app should use
        const std::string           web_url;            // full url to base page (i.e. http://<ip>:<port>/)
        Pistache::Http::Endpoint    web_app;            // the web app object
        bool                        is_running;         // true when web app is running

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Function called by constructor to help setup all the app's webpages
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes setupSites();


        /**
         * @brief Prints full urls listed in WebAppUrls
         */
        void printUrls() const;

}; // end of WebApp class

}; // end of UI namespace

}; // end of RPI namespace

#endif
