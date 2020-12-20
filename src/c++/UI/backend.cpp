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
    // , web_app{}
    , is_running{false}
{
    if(setupSites() != ReturnCodes::Success) {
        cerr << "ERROR: Failed to setup web app" << endl;
    }
}

WebApp::~WebApp() {
    // make sure web app is killed
    stopWebApp();
}

/********************************************* Getters/Setters *********************************************/


/********************************************* Web UI Functions *********************************************/


void WebApp::startWebApp(const bool print_urls) {
    // print urls (if told to)
    if (print_urls) {
        printUrls();
    }

    // start running the web app
    is_running = true;
}

void WebApp::stopWebApp() {
    // causes issues trying to close web app if it is not open
    if (is_running) {
        is_running = false;
    }
}

/********************************************* Helper Functions ********************************************/

ReturnCodes WebApp::setupSites() {
    // Note: create copy of map's strings (needs rvalue ref, which cannot be made from a const)

    // redirect landing page to main page
    // WebAppUrls.at(WebAppUrlsNames::LANDING_PAGE) => WebAppUrls.at(WebAppUrlsNames::MAIN_PAGE)

    // actual main page
    // WebAppUrls.at(WebAppUrlsNames::MAIN_PAGE)

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

