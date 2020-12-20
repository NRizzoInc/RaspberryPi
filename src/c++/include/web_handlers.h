#ifndef RPI_WEB_HANDLERS_H
#define RPI_WEB_HANDLERS_H

// Standard Includes
#include <iostream>
#include <string>
#include <functional>

// Our Includes

// 3rd Party Includes
#include "pistache/endpoint.h" // for actually web app server
#include "pistache/router.h" // to be able to make routes

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
}; // end of HelloHander (hello world example w/o route -- single page)


/******************************************** Web/Route Functions *******************************************/


}; // end of Handlers namespace

}; // end of UI namespace

}; // end of RPI namespace

#endif
