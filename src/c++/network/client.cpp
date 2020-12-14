#include "client.h"

namespace Network {

TcpClient::TcpClient(const std::string& ip_addr, const int port_num)
    : Packet{}
    , client_sock_fd{-1}        // init to invalid
    , server_ip{ip_addr}        // ip address to try to reach server
    , server_port{port_num}     // port the client tries to reach the server at
    , should_exit{false}        // not ready to end connection yet
{
    // stub
}

TcpClient::~TcpClient() {
    // stub
}

} // end of Network namespace