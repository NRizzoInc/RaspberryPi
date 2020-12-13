#include "server.h"

namespace RPI {


/********************************************** Constructors **********************************************/

TcpServer::TcpServer(const int port_num)
    : listen_sock_fd{-1}            // init to invalid
    , data_sock_fd{-1}              // init to invalid
    , ip_addr{}                     // empty string bc no client yet
    , listen_port{port_num}         // wait to accept connections at this port
    , end_conn{false}               // not ready to end connection yet
{
    // stub
}

TcpServer::~TcpServer() {
    // stub
}


/********************************************* Getters/Setters *********************************************/


/********************************************* Server Functions ********************************************/


/********************************************* Helper Functions ********************************************/

} // end of RPI namespace
