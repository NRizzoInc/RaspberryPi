#ifndef RPI_SERVER_H
#define RPI_SERVER_H

// Standard Includes
#include <iostream>
#include <string>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

// Our Includes

// 3rd Party Includes

namespace RPI {

class TcpServer {
    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a new Tcp Server object
         * @param port_num The port number for the listen port
         */
        TcpServer(const int port_num);
        virtual ~TcpServer();

        /********************************************* Getters/Setters *********************************************/

        /********************************************* Server Functions ********************************************/


    private:
        /******************************************** Private Variables ********************************************/

        int listen_sock_fd;     // tcp socket file descriptor to wait to accept connections with client
        int data_sock_fd;       // tcp socket file descriptor to communciate data with client
        std::string ip_addr;    // ip address of connected client
        int listen_port;        // port number of client
        bool end_conn;          // true if should exit/stop connection

        /********************************************* Helper Functions ********************************************/

}; // end of TcpServer class


} // end of RPI namespace

#endif
