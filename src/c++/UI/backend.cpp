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
        return;
    }
    web_app.shutdown();
}

/******************************************** Web/Route Functions *******************************************/

ReturnCodes WebApp::setupSites() {
    // see https://github.com/pistacheio/pistache/blob/master/examples/rest_server.cc
    // setup web app options
    auto opts = Pistache::Http::Endpoint::options().threads(1);
    web_app.init(opts);

    // main page
    Pistache::Rest::Routes::Get(
        web_app_router,
        WebAppUrls.at(WebAppUrlsNames::MAIN_PAGE),
        Pistache::Rest::Routes::bind(&WebApp::recvMainData, this)
    );


    // shutdown/close page
    Pistache::Rest::Routes::Get(
        web_app_router,
        WebAppUrls.at(WebAppUrlsNames::SHUTDOWN_PAGE),
        Pistache::Rest::Routes::bind(&WebApp::handleShutdown, this)
    );

    // use the default routing handler to manage the routing of multiple sites/routes
    web_app.setHandler(web_app_router.handler());

    return ReturnCodes::Success;
}

void WebApp::recvMainData(
    __attribute__((unused)) const Pistache::Rest::Request& req,
    Pistache::Http::ResponseWriter res
) {
    // TODO: actually parse request to get data to send via client
    res.send(Pistache::Http::Code::Ok, "Successfully received data!\n");
}

/// redirect function (TODO)
// void Redirect(
//     const std::string& redirect_url,
//     const Pistache::Rest::Request& req,
//     Pistache::Http::ResponseWriter res
// ) {
//     res.send(Pistache::Http::Code::Ok, {"Redirecting to " + redirect_url});
// }

void WebApp::handleShutdown(
    __attribute__((unused)) const Pistache::Rest::Request& req,
    Pistache::Http::ResponseWriter res
) {
    client_ptr->setExitCode(true);
    res.send(Pistache::Http::Code::Ok, "Stopping Web App Server\n");
}

/********************************************* Helper Functions ********************************************/


void WebApp::printUrls() const {
    cout << "Web App's Urls: " << endl;
    for(auto& url : WebAppUrls) {
        cout << web_url_root << url.second << endl;
    }
}

}; // end of UI namespace

}; // end of RPI namespace

