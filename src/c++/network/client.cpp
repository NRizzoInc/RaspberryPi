#include "client.h"

namespace Network {

using std::cout;
using std::cerr;
using std::endl;


TcpClient::TcpClient(const std::string& ip_addr, const int port_num)
    : Packet{}
    , NetCommon{}
    , client_sock_fd{-1}        // init to invalid
    , server_ip{ip_addr}        // ip address to try to reach server
    , server_port{port_num}     // port the client tries to reach the server at
{
    // stub
}

TcpClient::~TcpClient() {
    // stub
}

} // end of Network namespace