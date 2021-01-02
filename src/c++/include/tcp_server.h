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

namespace RPI {
namespace Network {

class TcpServer : public TcpBase {
    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a new Tcp Server object
         * @param ctrl_data_port The port number for the server to listen for control packets from client
         * @param cam_send_port The port number for the camera data to be sent to client
         * @param should_init False: do not init (most likely bc should run client)
         */
        TcpServer(
            const int ctrl_data_port,
            const int cam_send_port,
            const bool should_init
        );
        virtual ~TcpServer();

        /********************************************* Getters/Setters *********************************************/


        /********************************************* Server Functions ********************************************/

        /**
         * @brief Accept a new connection and stores the IP of the client
         * @return Success if connected successfully
         * @return Error if failed to connect to client
         */
        ReturnCodes acceptClient();

    protected:

        /**
         * @brief Starts up a non-blocking function to start the server listener
         * @param print_data Should received data be printed?
         */
        virtual void netAgentFn(const bool print_data) override;

        /**
         * @brief Starts up a non-blocking function to send video frames from camera to client
         */
        virtual void VideoStreamHandler() override;


    private:
        /******************************************** Private Variables ********************************************/

        int             listen_sock_fd;     // tcp socket file descriptor to wait to accept connections with client
        int             ctrl_sock_fd;       // tcp socket file descriptor to recv control data from client
        std::string     client_ip;          // ip address of connected client
        int             ctrl_data_port;   // port number for socket receiving control data from client

        // camera vars
        int             cam_listen_sock_fd; // tcp file descriptor to wait for camera conn
        int             cam_data_sock_fd;   // tcp file descriptor to transfer camera data
        int             cam_data_port;      // port number for camera data transfer to client

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Creates the socket, bind it & sets options. Starts listening for connections.
         * @return Error as soon as any of the operations it performs fails. Success if no issues
         */
        ReturnCodes initSock() override;

        /**
         * @brief Function called at end of running server to close the sockets
         */
        void quit() override;


}; // end of TcpServer class


} // end of Network namespace

}; // end of RPI namespace

#endif
