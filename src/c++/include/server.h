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
#include "packet.h"
#include "net_common.h"

// 3rd Party Includes

namespace Network {

class TcpServer : public Packet, public NetCommon {
    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a new Tcp Server object
         * @param port_num The port number for the listen port
         * @param should_init False: do not init (most likely bc is client)
         */
        TcpServer(const int port_num, const bool should_init=true);
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
         * @brief Receives data from remote host.
         * @param buf buffer where the received data is stored
         * @return number of bytes received (-1 if error occurred)
         */
        int recvData(char* buf);

        /**
         * @brief Send data to remote host.
         * @param buf pointer to the buffer where the data to be sent is stored
         * @param size_to_tx size to transmit
         * @return number of bytes sent (-1 if error occurred)
         */
        int sendData(const char* buf, const size_t size_to_tx);

        /**
         * @brief Blocking function to start the server listener
         * @param print_data Should received data be printed?
         */
        void runServer(const bool print_data);

    private:
        /******************************************** Private Variables ********************************************/

        int             listen_sock_fd;     // tcp socket file descriptor to wait to accept connections with client
        int             data_sock_fd;       // tcp socket file descriptor to communciate data with client
        std::string     client_ip;          // ip address of connected client
        int             listen_port;        // port number of client

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Set the socket options and bind.
         * @return Error as soon as any of the operations it performs fails. Success if no issues
         */
        ReturnCodes optionsAndBind();

        /**
         * @brief Function called by the destructor to close the sockets
         */
        void quit();


}; // end of TcpServer class


} // end of Network namespace

#endif
