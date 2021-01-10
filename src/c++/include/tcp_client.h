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
#include <chrono> // send packet every right before server timeout
#include <mutex>
#include <condition_variable> // block with mutex until new data set
#include <atomic>

// Our Includes
#include "constants.h"
#include "tcp_base.h"

// 3rd Party Includes

namespace RPI {
namespace Network {

class TcpClient : public TcpBase {
    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a new Tcp Client object
         * @param ip_addr The ip address of the server
         * @param ctrl_port_num The port number the server is waiting to accept control packet connections on
         * @param cam_port_num The port for the camera data connection
         * @param should_init False: do not init (most likely bc should run server)
         */
        TcpClient(
            const std::string& ip_addr,
            const int ctrl_port_num,
            const int cam_port_num,
            const bool should_init
        );
        virtual ~TcpClient();

        /********************************************* Getters/Setters *********************************************/

        /**
         * @brief Wraps the base updater with a lock & notifies client to send it
         * @param updated_pkt The up to date packet to send
         * @return ReturnCodes Success if it worked
         * @note Get the current pkt with getCurrentPkt()
         */
        ReturnCodes updatePkt(const CommonPkt& updated_pkt) override;


        /********************************************* Client Functions ********************************************/

        /**
         * @brief Sends a reset packet to the server that resets most things to default
         * @note There might be some special cases that it doesnt use the defaults for
         * @return ReturnCodes Success if no issues
         */
        virtual ReturnCodes sendResetPkt() override;

    protected:

        /**
         * @brief Starts up a non-blocking function to start sending messages to server
         * @param print_data Should received data be printed?
         */
        virtual void ControlLoopFn(const bool print_data) override;

        /**
         * @brief Starts up a non-blocking function to recv video frames from server camera 
         * @param print_data Should received data be printed?
         */
        virtual void VideoStreamHandler(const bool print_data) override;

    private:
        /******************************************** Private Variables ********************************************/

        int                         ctrl_data_sock_fd;  // tcp socket file descriptor that sends control data to server
        std::string                 server_ip;          // ip address of the server
        int                         ctrl_data_port;     // port number to send control data to the server
        std::atomic_bool            pkt_ready;          // alert send cv to unlock
        std::mutex                  data_mutex;
        std::condition_variable     has_new_msg;        // true if client needs to tell the server something

        // camera vars
        int                         cam_data_sock_fd;   // tcp file descriptor for camera data from server
        int                         cam_data_port;      // port number for getting camera from server

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Creates the socket, sets options, & makes everything ready to connect to server.
         * @return Error as soon as any of the operations it performs fails. Success if no issues
         */
        ReturnCodes initSock() override;

        /**
         * @brief Responsible for connecting to server prior to communicating with server
         * @param sock_fd The socket file descriptor to open the connection from (reference so can close on fail)
         * @param ip The ip to connect to
         * @param port The port to connect to
         * @param conn_desc Description of the connection (i.e. "camera" or "control")
         * @return ReturnCodes 
         */
        ReturnCodes connectToServer(int& sock_fd, const std::string& ip, const int port, const std::string& conn_desc);

        /**
         * @brief Function called at the end of running client to close the sockets
         */
        void quit() override;

}; // end of TcpClient class


} // end of Network namespace

}; // end of RPI namespace

#endif
