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
#include <atomic> // for memset

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
         * @param srv_data_port_num The port to send server data to client on
         * @param should_init False: do not init (most likely bc should run client)
         * @param verbosity If true, will print more information that is strictly necessary
         */
        TcpServer(
            const int ctrl_data_port,
            const int cam_send_port,
            const int srv_data_port_num,
            const bool should_init,
            const bool verbosity=false
        );
        virtual ~TcpServer();

        /********************************************* Getters/Setters *********************************************/


        /********************************************* Server Functions ********************************************/

        /**
         * @brief Accept a new connection and stores the IP of the client
         * @param listen_sock_fd the sock file descriptor that is listening such that it can accepting client
         * @param data_sock_fd the sock file descriptor to assigned the accepted client to
         * @param conn_desc A string stating the purpose of the connection (i.e. camera/control)
         * @param port The port to accept the client connection on
         * @note the socket file descriptors are references so they can be modified
         * @return Success if connected successfully
         * @return Error if failed to connect to client
         */
        ReturnCodes acceptClient(
            int& listen_sock_fd,
            int& data_sock_fd,
            const std::string& conn_desc,
            const int port
        );

        /**
         * @brief Sends a reset packet to the client that resets most things to default
         * @note There might be some special cases that it doesnt use the defaults for
         * @return ReturnCodes Success if no issues
         */
        virtual ReturnCodes sendResetPkt() override;

    protected:

        /**
         * @brief Starts up a non-blocking function to start the server listener
         * @param print_data Should received data be printed?
         */
        virtual void ControlLoopFn(const bool print_data) override;

        /**
         * @brief Starts up a non-blocking function to send video frames from camera to client
         */
        virtual void VideoStreamHandler() override;

        /**
         * @brief Starts up a non-blocking function to send server data to the client (not-camera related) 
         */
        virtual void ServerDataHandler(const bool print_data) override;

    private:
        /******************************************** Private Variables ********************************************/

        // misc vars
        std::atomic_bool         close_conns;         // true if a connection has been closed (meaning all should)

        // control vars
        int                      ctrl_listen_sock_fd; // tcp socket file descriptor to accept connections from client
        int                      ctrl_data_sock_fd;   // tcp socket file descriptor to recv control data from client
        std::string              client_ip;           // ip address of connected client
        const int                ctrl_data_port;      // port number for socket receiving control data from client

        // camera vars
        int                      cam_listen_sock_fd;  // tcp file descriptor to wait for camera conn
        int                      cam_data_sock_fd;    // tcp file descriptor to transfer camera data
        const int                cam_data_port;       // port number for camera data transfer to client

        // server data vars
        int                      srv_data_listen_sock_fd;   // tcp file descriptor to wait for server data conn
        int                      srv_data_sock_fd;          // tcp file descriptor to transfer server data to client
        const int                srv_data_port;             // port number for server data transfer to client

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
