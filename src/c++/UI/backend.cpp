#include "backend.h"


namespace RPI {

namespace UI {

using std::cout;
using std::cerr;
using std::endl;
using nlohmann::json;

// get paths (operator/ is used for joining)
const fs::path      CURR_DIR             {fs::path{__FILE__}.parent_path()};
const fs::path      FRONTEND_DIR         {CURR_DIR / "frontend"};
const fs::path      HTML_DIR             {FRONTEND_DIR / "html"};
const fs::path      STATIC_DIR           {FRONTEND_DIR / "static"};

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

ReturnCodes WebApp::stopWebApp() {
    // causes issues trying to close web app if it is not open
    if (is_running) {
        is_running = false;
        web_app.shutdown();
    }
    return ReturnCodes::Success;
}

/******************************************** Web/Route Functions *******************************************/

ReturnCodes WebApp::setupSites() {
    // see https://github.com/pistacheio/pistache/blob/master/examples/rest_server.cc
    // setup web app options
    auto opts = Pistache::Http::Endpoint::options()
        .threads(1)
        // prevent bug "EADDRINUSE (Address already in use)" after quick stop/start
        .flags(Pistache::Tcp::Options::ReuseAddr)
        ;
    web_app.init(opts);

    // main page
    Pistache::Rest::Routes::Get(
        web_app_router,
        WebAppUrls.at(WebAppUrlsNames::MAIN_PAGE),
        Pistache::Rest::Routes::bind(&WebApp::serveMainPage, this)
    );
    Pistache::Rest::Routes::Post(
        web_app_router,
        WebAppUrls.at(WebAppUrlsNames::MAIN_PAGE),
        Pistache::Rest::Routes::bind(&WebApp::recvMainData, this)
    );

    // video stream pages
    Pistache::Rest::Routes::Get(
        web_app_router,
        WebAppUrls.at(WebAppUrlsNames::CAM_PAGE),
        Pistache::Rest::Routes::bind(&WebApp::handleVidReq, this)
    );
    Pistache::Rest::Routes::Get(
        web_app_router,
        WebAppUrls.at(WebAppUrlsNames::CAM_SETTINGS),
        Pistache::Rest::Routes::bind(&WebApp::handleCamSettingReq, this)
    );


    // shutdown/close page
    Pistache::Rest::Routes::Get(
        web_app_router,
        WebAppUrls.at(WebAppUrlsNames::SHUTDOWN_PAGE),
        Pistache::Rest::Routes::bind(&WebApp::handleShutdown, this)
    );

    // static resources (if invalid path, might be static resource, if not handles it)
    web_app_router.addNotFoundHandler(Pistache::Rest::Routes::bind(&WebApp::serveStaticResources, this));

    // use the default routing handler to manage the routing of multiple sites/routes
    web_app.setHandler(web_app_router.handler());

    return ReturnCodes::Success;
}

void WebApp::serveMainPage(
    __attribute__((unused)) const Pistache::Rest::Request& req,
    Pistache::Http::ResponseWriter res
) {
    // guide: https://github.com/pistacheio/pistache/blob/master/src/common/description.cc
    const fs::path MAIN_PAGE_PATH {HTML_DIR / "index.html"};
    Pistache::Http::serveFile(res, MAIN_PAGE_PATH.c_str());
}

// cannot serve directory, have to manually server files:
// https://github.com/pistacheio/pistache/blob/1df04a35cd54f476dc788c0175f37395df701f7d/examples/http_server.cc#L140-L146
void WebApp::serveStaticResources(
    const Pistache::Rest::Request& req,
    Pistache::Http::ResponseWriter res
) {
    const fs::path  js_path     {STATIC_DIR / "js"};
    const fs::path  ext_path    {STATIC_DIR / "extern"};
    const fs::path  css_path    {STATIC_DIR / "stylesheets"};
    const fs::path  images_path {STATIC_DIR / "images"};

    // determine what resource is being asked for & manually serve it
    const std::string req_page {req.resource()};
    const std::string res_page_name {Helpers::findFilename(req_page)};

    // security check (first make sure it is not going back more than one dir -- tryna access actual filesystem)
    // not have to specific MIME type as defined by extern/pistache/include/pistache/mime.h
    // or see: http://pistache.io/guide/#response-streaming (ctrl+f "MIME types")
    if (Helpers::contains(req_page, "../..")) {
        res.send(Pistache::Http::Code::Moved_Permanently, "Invalid Path!\n");
    } 
    // serve the requested stylesheet
    // only serve file if valid => if not reach end of fn
    else if (Helpers::contains(req_page, "stylesheets")) {
        const fs::path serve_file {css_path / res_page_name};
        if (fs::exists(serve_file)) {
            Pistache::Http::serveFile(
                res,
                serve_file.string(),
                Pistache::Http::Mime::MediaType(
                    Pistache::Http::Mime::Type::Text, // main type
                    Pistache::Http::Mime::Subtype::Css // sub type
                )
            );
            return;
        }
    } 
    // serve the requested images
    // only serve file if valid => if not reach end of fn
    else if (Helpers::contains(req_page, "images")) {
        const fs::path serve_file {images_path / res_page_name};
        if (fs::exists(serve_file)) {
            Pistache::Http::serveFile(
                res,
                serve_file.string(),
                Pistache::Http::Mime::MediaType(
                    Pistache::Http::Mime::Type::Application, // main type
                    Pistache::Http::Mime::Subtype::Png // sub type
                )
            );
            return;
        }
    } else if (Helpers::contains(req_page, "js")) {
        // serve the requested js file
        // only serve file if valid => if not reach end of fn
        const fs::path serve_file {js_path / res_page_name};
        if (fs::exists(serve_file)) {
            Pistache::Http::serveFile(
                res,
                serve_file.string(),
                Pistache::Http::Mime::MediaType(
                    Pistache::Http::Mime::Type::Application, // main type
                    Pistache::Http::Mime::Subtype::Javascript // sub type
                )
            );
            return;
        }
    } else if (Helpers::contains(req_page, "extern")) {
        const fs::path full_rel_path = Helpers::findPathAfter(req_page, "/static/extern/");
        const fs::path serve_file {ext_path / full_rel_path};
        if (fs::exists(serve_file)) {
            Pistache::Http::serveFile(
                res,
                serve_file.string(),
                Pistache::Http::Mime::MediaType(
                    Pistache::Http::Mime::Type::Text, // main type
                    Pistache::Http::Mime::Subtype::Css // sub type
                )
            );
            return;
        }
    } else if (Helpers::contains(req_page,  "../fonts/fontawesome")) {
        // special path for fontawesome external library
        // -- should not be security risk since requested file MUST be below and not up
        const fs::path serve_file {ext_path / "font-awesome-4.7.0" / "fonts" / res_page_name};
        if (fs::exists(serve_file)) {
            Pistache::Http::serveFile(
                res,
                serve_file.string(),
                Pistache::Http::Mime::MediaType(
                    Pistache::Http::Mime::Type::Application, // main type
                    Pistache::Http::Mime::Subtype::Bmp // sub type
                )
            );
            return;
        }
    }

    // bad/invalid path to reach this point
    res.headers().add<Pistache::Http::Header::Location>(req_page);
    res.send(Pistache::Http::Code::Moved_Permanently, "Invalid Path!\n");
}

void WebApp::recvMainData(
    const Pistache::Rest::Request& req,
    Pistache::Http::ResponseWriter res
) {
    try {
        const std::string& req_str {req.body()}; 
        const RPI::Network::CommonPkt updated_pkt {client_ptr->readCmnPkt(req_str.c_str(), req_str.size(), false)};
        client_ptr->updatePkt(updated_pkt);
        res.send(Pistache::Http::Code::Ok, "Successfully received data!\n");
    } catch (std::exception& err) {
        cout << "ERROR: Bad web app data: " << err.what() << endl;
        res.send(Pistache::Http::Code::Bad_Request, "Bad Data Sent!\n");
    }
}

void WebApp::handleVidReq(
    __attribute__((unused)) const Pistache::Rest::Request& req,
    Pistache::Http::ResponseWriter res
) {
    try {
        // stores pixel data
        const std::vector<unsigned char>& frame   { client_ptr->getLatestCamFrame() };
        const std::size_t img_size                { frame.size() };
        const char* frame_buf                     { img_size > 0 ? (char*)frame.data() : "" };

        // actually send the pixel data back to GET request
        res.send(
            Pistache::Http::Code::Ok,
            frame_buf, img_size,
            Pistache::Http::Mime::MediaType(
                Pistache::Http::Mime::Type::Image, // main type
                Pistache::Http::Mime::Subtype::Jpeg // sub type
            )
        );

    } catch (std::exception& err) {
        constexpr auto err_str {"ERROR: Handling web app video data"};
        cout << err_str << ": " << err.what() << endl;
        res.send(Pistache::Http::Code::Bad_Request, err_str);
    }
}

void WebApp::handleCamSettingReq(
    __attribute__((unused)) const Pistache::Rest::Request& req,
    Pistache::Http::ResponseWriter res
) {
    try {
        // https://github.com/nlohmann/json#json-as-first-class-data-type
        // have to double wrap {{}} to get it to work (each key-val needs to be wrapped)
        // key-values are seperated by commas not ':'
        json cam_settings {
            {"fps", Constants::Camera::VID_FRAMERATE},
            {"height", Constants::Camera::FRAME_HEIGHT},
            {"width", Constants::Camera::FRAME_WIDTH},
        };

        // actually send the pixel data back to GET request
        res.send(
            Pistache::Http::Code::Ok,
            cam_settings.dump(),
            Pistache::Http::Mime::MediaType(
                Pistache::Http::Mime::Type::Application, // main type
                Pistache::Http::Mime::Subtype::Json // sub type
            )
        );

    } catch (std::exception& err) {
        constexpr auto err_str {"ERROR: Sending camera settings"};
        cout << err_str << ": " << err.what() << endl;
        res.send(Pistache::Http::Code::Bad_Request, err_str);
    }
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
        // make sure path starts with '/' so url looks like http://<ip>:<port>/
        const std::string suffix_path {
            Helpers::startsWith(url.second, "/") ? 
                url.second :
                std::string{"/"} + url.second
        };
        const std::string main_comment {
            url.first == WebAppUrlsNames::MAIN_PAGE ? " -- use this main page" : ""};
        cout << web_url_root << suffix_path << main_comment << endl;
    }
}

}; // end of UI namespace

}; // end of RPI namespace

