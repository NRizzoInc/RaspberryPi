#ifndef RPI_BACKEND_H
#define RPI_BACKEND_H

// Standard Includes
#include <iostream>
#include <string>
#include <experimental/filesystem> // to get path to html file

// Our Includes
#include "string_helpers.hpp"
#include "constants.h"
#include "tcp_base.h" // shared_ptr to base class (for updatePkt())
#include "web_handlers.h"

// 3rd Party Includes
#include <json.hpp>
#include "pistache/endpoint.h" // for actually web app server
#include "pistache/router.h" // to be able to make routes

namespace RPI {

namespace UI {

// filesystem namespace shortened for convenience bc super long & experimental
namespace fs = std::experimental::filesystem;

// TODO: convert to variable that can be changed with input
const std::string   URL_BASE_IP          {"http://127.0.0.1"};

// used by WebAppUrls as keys to select specific urls
enum class WebAppUrlsNames {
    // LANDING_PAGE, //TODO: get redirect to work
    MAIN_PAGE,
    SHUTDOWN_PAGE,
    STATIC,
};

// contains actual urls as values
const std::unordered_map<WebAppUrlsNames, std::string> WebAppUrls {
    // {WebAppUrlsNames::LANDING_PAGE, "/"}, //TODO: get redirect to work
    {WebAppUrlsNames::MAIN_PAGE, "/RPI-Client"},
    {WebAppUrlsNames::SHUTDOWN_PAGE, "/Shutdown"},
    {WebAppUrlsNames::STATIC, "../static"}, // from perspective of html file, static is one back
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
        const std::string           web_url_root;       // full url to base page (i.e. http://<ip>:<port>/)
        Pistache::Http::Endpoint    web_app;            // the web app object
        Pistache::Rest::Router      web_app_router;     // default route handler for creation & routing of multi sites
        bool                        is_running;         // true when web app is running

        /******************************************** Web/Route Functions *******************************************/

        /**
         * @brief Serves the html file so user can submit form containing jsonified data for client
         * @param req The request object
         * @param res The response object
         * @note See https://github.com/pistacheio/pistache/blob/master/src/common/description.cc
         */
        void serveMainPage(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter res);


        /**
         * @brief Serves the frontend static resources (i.e js, css, images, etc)
         * @param req The request object
         * @param res The response object
         * @note See https://github.com/pistacheio/pistache/blob/master/src/common/description.cc
         */
        void serveStaticResources(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter res);


        /**
         * @brief Responsible for managing the data that comes in when user utilizes the main page of the web app
         * (updates packet for client to be sent to server)
         */
        void recvMainData(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter res);

        // TODO: Get redirect to work (hard to do function generator/flexible with this bind)
        ///**
        // * @brief Create a route function that will redirect to another page
        // * @param redirect_url The full url of where to redirect to
        // */
        //void Redirect(
        //    const std::string& redirect_url,
        //    const Pistache::Rest::Request& req,
        //    Pistache::Http::ResponseWriter res
        //);

        /**
         * @brief Responsible for closing web server & telling client to stop
         * 
         */
        void handleShutdown(const Pistache::Rest::Request& req, Pistache::Http::ResponseWriter res);

        /**
         * @brief Function called by constructor to help setup all the app's webpages
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes setupSites();

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Prints full urls listed in WebAppUrls
         */
        void printUrls() const;

}; // end of WebApp class

}; // end of UI namespace

}; // end of RPI namespace

#endif
