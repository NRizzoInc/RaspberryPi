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

// Our Includes
#include "constants.h"

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
        
        /**
         * @brief Sets the exit code. 
         * @note Useful for terminating runServer() from main thread
         */
        virtual ReturnCodes setExitCode(const bool new_exit) const;

        /**
         * @brief Get the current exit code status
         * @return true TcpServer is ready to or should exit
         * @return false TcpServer is still running and not ready to exit
         */
        virtual bool getExitCode() const;


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

    private:
        /******************************************** Private Variables ********************************************/

        int             listen_sock_fd;     // tcp socket file descriptor to wait to accept connections with client
        int             data_sock_fd;       // tcp socket file descriptor to communciate data with client
        std::string     client_ip;          // ip address of connected client
        int             listen_port;        // port number of client
        mutable bool    should_exit;        // true if should exit/stop connection

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


} // end of RPI namespace

#endif
