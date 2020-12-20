#ifndef RPI_WEB_HANDLERS_H
#define RPI_WEB_HANDLERS_H

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

namespace Handlers {

/**
 * @brief Simple hello world test handler
 * @note Have to override the "onRequest()" and "clone()" functions to derive from handler
 * @example https://github.com/pistacheio/pistache/blob/master/examples/hello_server.cc
 */
class HelloHandler : public Pistache::Http::Handler {
    public:

        // takes care of required "clone()" function (only works for simple cases)
        HTTP_PROTOTYPE(HelloHandler);

        void onRequest(
            const Pistache::Http::Request& request,
            Pistache::Http::ResponseWriter response
        ) override;
};


}; // end of Handlers namespace

}; // end of UI namespace

}; // end of RPI namespace

#endif
