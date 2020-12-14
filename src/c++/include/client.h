#ifndef RPI_CLIENT_H
#define RPI_CLIENT_H

// Standard Includes
#include <iostream>
#include <string>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // for socket close()
#include <cstring> // for memset

// Our Includes
#include "constants.h"
#include "packet.h"
#include "net_common.h"

// 3rd Party Includes

namespace Network {

class TcpClient : public Packet, public NetCommon {
    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a new Tcp Client object
         * @param ip_addr The ip address of the server
         * @param port_num The port number of the server
         */
        TcpClient(const std::string& ip_addr, const int port_num);
        virtual ~TcpClient();

        /********************************************* Getters/Setters *********************************************/


        /********************************************* Server Functions ********************************************/


    private:
        /******************************************** Private Variables ********************************************/

        int             client_sock_fd;     // tcp socket file descriptor to wait to accept connections with client
        std::string     server_ip;          // ip address of the server
        int             server_port;        // port number of the server

        /********************************************* Helper Functions ********************************************/


}; // end of TcpClient class


} // end of Network namespace

#endif
