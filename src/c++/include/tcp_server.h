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

        /**
         * @brief Set the latest frame from the camera video stream (and set bool saying there is new data)
         * @return Success if no issues
         * @note Bool is used to prevent over sending of the same traffic over and over again
         */
        virtual ReturnCodes setLatestCamFrame(const std::vector<unsigned char>& new_frame) override;

        /**
         * @brief Get the latest frame from the camera video stream (and set new atomic flag to false)
         * @return Reference to the char buffer in the form of a char vector
         */
        virtual const std::vector<unsigned char>& getLatestCamFrame() const override;


    private:
        /******************************************** Private Variables ********************************************/

        int                      ctrl_listen_sock_fd; // tcp socket file descriptor to accept connections from client
        int                      ctrl_data_sock_fd;   // tcp socket file descriptor to recv control data from client
        std::string              client_ip;           // ip address of connected client
        int                      ctrl_data_port;      // port number for socket receiving control data from client

        // camera vars
        int                      cam_listen_sock_fd;  // tcp file descriptor to wait for camera conn
        int                      cam_data_sock_fd;    // tcp file descriptor to transfer camera data
        int                      cam_data_port;       // port number for camera data transfer to client
        std::condition_variable  cam_data_cv;         // notify in order for server to send camera data to client
        std::mutex               cam_data_mutex;      // mutex to lock when accessing the camera data
        mutable std::atomic_bool has_new_cam_data;    // true if there is new data to send
        std::atomic_bool         close_conns;         // true if a connection has been closed (meaning all should)

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
