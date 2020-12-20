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
    , web_url_root{std::string(URL_BASE_IP) + ":" + std::to_string(web_port)}
    , web_app{Pistache::Address{Pistache::Ipv4::any(), Pistache::Port(web_port)}}
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
    web_app.serveThreaded();
}

void WebApp::stopWebApp() {
    // causes issues trying to close web app if it is not open
    if (is_running) {
        is_running = false;
    }
}

/********************************************* Helper Functions ********************************************/

ReturnCodes WebApp::setupSites() {
    // setup web app options
    auto opts = Pistache::Http::Endpoint::options().threads(1);
    web_app.init(opts);

    // redirect landing page to main page
    // WebAppUrls.at(WebAppUrlsNames::LANDING_PAGE) => WebAppUrls.at(WebAppUrlsNames::MAIN_PAGE)

    // actual main page
    // WebAppUrls.at(WebAppUrlsNames::MAIN_PAGE)
    web_app.setHandler(std::make_shared<Handlers::HelloHandler>());

    return ReturnCodes::Success;
}

void WebApp::printUrls() const {
    cout << "Web App's Urls: " << endl;
    for(auto& url : WebAppUrls) {
        cout << web_url_root << url.second << endl;
    }
}

}; // end of UI namespace

}; // end of RPI namespace

