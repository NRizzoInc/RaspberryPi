#include "web_handlers.h"

using std::cout;
using std::cerr;
using std::endl;


namespace RPI {

namespace UI {

namespace Handlers {

void HelloHandler::onRequest(
    __attribute__((unused)) const Pistache::Http::Request& request,
    Pistache::Http::ResponseWriter response
) {
    response.send(Pistache::Http::Code::Ok, "Hello, World");
}

/******************************************** Web/Route Functions *******************************************/


}; // end of Handlers namespace

}; // end of UI namespace

}; // end of RPI namespace

