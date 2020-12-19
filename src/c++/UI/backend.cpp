#include "backend.h"


namespace RPI {

namespace UI {

using std::cout;
using std::cerr;
using std::endl;

/********************************************** Constructors **********************************************/

WebApp::WebApp(const std::shared_ptr<RPI::Network::TcpBase> tcp_client, const int port)
    : client_ptr{tcp_client}
    , web_port{port}
    , web_url{std::string(URL_BASE) + "/" + std::to_string(web_port)}
    , web_app{}
{
    if(setupSites() != ReturnCodes::Success) {
        cerr << "ERROR: Failed to setup web app" << endl;
    }
}

WebApp::~WebApp() {
    // stub
}

/********************************************* Getters/Setters *********************************************/


void WebApp::startWebApp(const bool print_urls) {
    // print urls (if told to)
    if (print_urls) {
        printUrls();
    }

    // start running the web app
    web_app.port(web_port).multithreaded().run();
}

/********************************************* Helper Functions ********************************************/

ReturnCodes WebApp::setupSites() {
    // Note: create copy of map's strings (needs rvalue ref, which cannot be made from a const)

    // redirect landing page to main page
    web_app.route_dynamic(std::string{WebAppUrls.at(WebAppUrlsNames::LANDING_PAGE)})(
        [&](__attribute__((unused)) const crow::request &req, crow::response &res) {
            // to redirect have to proivde full url: https://github.com/ipkn/crow/issues/346
            res.redirect(web_url + std::string{WebAppUrls.at(WebAppUrlsNames::MAIN_PAGE)});
        }
    );

    web_app.route_dynamic(std::string{WebAppUrls.at(WebAppUrlsNames::MAIN_PAGE)})(
        [&]() {
            return "Main Page";
        }
    );

    return ReturnCodes::Success;
}

void WebApp::printUrls() const {
    cout << "Web App's Urls: " << endl;
    for(auto& url : WebAppUrls) {
        cout << web_url << url.second << endl;
    }
}

}; // end of UI namespace

}; // end of RPI namespace

