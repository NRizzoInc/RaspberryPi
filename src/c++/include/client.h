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

namespace Network {

class TcpClient : public TcpBase {
    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a new Tcp Client object
         * @param ip_addr The ip address of the server
         * @param port_num The port number of the server
         * @param should_init False: do not init (most likely bc should run server)
         */
        TcpClient(const std::string& ip_addr, const int port_num, const bool should_init);
        virtual ~TcpClient();

        /********************************************* Getters/Setters *********************************************/

        /**
         * @brief Wraps the base updater with a lock & notifies client to send it
         * @param updated_pkt The up to date packet to send
         * @return ReturnCodes Success if it worked
         */
        ReturnCodes updatePkt(const CommonPkt& updated_pkt) override;


        /********************************************* Client Functions ********************************************/

        /**
         * @brief Blocking function to start the client messager to server
         * @param print_data Should received data be printed?
         */
        void runNetAgent(const bool print_data) override;

    private:
        /******************************************** Private Variables ********************************************/

        int                         client_sock_fd;     // tcp socket file descriptor to wait to accept connections with client
        std::string                 server_ip;          // ip address of the server
        int                         server_port;        // port number of the server
        std::atomic_bool            is_first_msg;       // need to send an initization message for first packet
        std::mutex                  data_mutex;
        std::condition_variable     has_new_msg;        // true if client needs to tell the server something

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Creates the socket, sets options, & makes everything ready to connect to server.
         * @return Error as soon as any of the operations it performs fails. Success if no issues
         */
        ReturnCodes initSock() override;

        /**
         * @brief Responsible for connecting to server prior to communicating with server
         * 
         * @return ReturnCodes 
         */
        ReturnCodes connectToServer();

        /**
         * @brief Function called by the destructor to close the sockets
         */
        void quit() override;

}; // end of TcpClient class


} // end of Network namespace

#endif
