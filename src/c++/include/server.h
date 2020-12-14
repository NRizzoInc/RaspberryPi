#ifndef RPI_SERVER_H
#define RPI_SERVER_H

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
#include "tcp_base.h"

// 3rd Party Includes

namespace Network {

class TcpServer : public TcpBase {
    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a new Tcp Server object
         * @param port_num The port number for the listen port
         * @param should_init False: do not init (most likely bc should run client)
         */
        TcpServer(const int port_num, const bool should_init);
        virtual ~TcpServer();

        /********************************************* Getters/Setters *********************************************/


        /********************************************* Server Functions ********************************************/

        /**
         * @brief Accept a new connection and stores the IP of the client
         * @return Success if connected successfully
         * @return Error if failed to connect to client
         */
        ReturnCodes acceptClient();

        /**
         * @brief Blocking function to start the server listener
         * @param print_data Should received data be printed?
         */
        void runNetAgent(const bool print_data) override;

    private:
        /******************************************** Private Variables ********************************************/

        int             listen_sock_fd;     // tcp socket file descriptor to wait to accept connections with client
        int             data_sock_fd;       // tcp socket file descriptor to communciate data with client
        std::string     client_ip;          // ip address of connected client
        int             listen_port;        // port number of client

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Creates the socket, bind it & sets options. Starts listening for connections.
         * @return Error as soon as any of the operations it performs fails. Success if no issues
         */
        ReturnCodes initSock() override;

        /**
         * @brief Function called by the destructor to close the sockets
         */
        void quit() override;


}; // end of TcpServer class


} // end of Network namespace

#endif
