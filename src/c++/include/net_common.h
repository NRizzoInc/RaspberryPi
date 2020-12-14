#ifndef RPI_NET_COMMON_H
#define RPI_NET_COMMON_H

// Standard Includes
#include <iostream>
#include <string>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h> // for socket close()
#include <cstring> // for memset
#include <cassert> // for assert

// Our Includes
#include "constants.h"
#include "packet.h"

// 3rd Party Includes

namespace Network {

/**
 * @brief Implements common features shared between server & client
 * 
 */
class NetCommon : public Packet {
    public:
        /********************************************** Constructors **********************************************/

        NetCommon();
        virtual ~NetCommon();

        /********************************************* Getters/Setters *********************************************/


        /**
         * @brief Sets the exit code. 
         * @note Useful for terminating runNetAgent() from main thread
         */
        virtual ReturnCodes setExitCode(const bool new_exit) const;

        /**
         * @brief Get the current exit code status
         * @return true TcpServer is ready to or should exit
         * @return false TcpServer is still running and not ready to exit
         */
        virtual bool getExitCode() const;

        /**
         * @brief Blocking function to start the server/client
         * @param print_data Should received data be printed?
         */
        virtual void runNetAgent(const bool print_data) = 0;

    protected:
        /****************************************** Shared Common Functions ****************************************/

        /**
         * @brief Get the Public Ip object and stores it in buf
         * @param buffer The buffer to hold the public ip address
         * @param buf_size The size of the buffer (>=16)
         * @credit: https://stackoverflow.com/a/3120382/13933174
         */
        void GetPublicIp(char* buffer, std::size_t buf_size) const;

        /**
         * @brief Creates a string of format: <ip>:<port>
         * @param ip The ip address
         * @param port The port number
         * @return The string containing the formatted address
         */
        std::string formatIpAddr(const std::string& ip, const int port) const;

        /**
         * @brief Receives data from remote host.
         * @param socket_fd The receiving socket's file descriptor
         * @param buf buffer where the received data is stored
         * @return number of bytes received (-1 if error occurred)
         */
        virtual int recvData(int socket_fd, char* buf);

        /**
         * @brief Send data to remote host.
         * @param socket_fd The receiving socket's file descriptor
         * @param buf pointer to the buffer where the data to be sent is stored
         * @param size_to_tx size to transmit
         * @return number of bytes sent (-1 if error occurred)
         */
        virtual int sendData(int socket_fd, const char* buf, const size_t size_to_tx);

        /**
         * @brief Creates the socket, bind it & sets options. Override to be called in constructor
         * @return Error as soon as any of the operations it performs fails. Success if no issues
         */
        virtual ReturnCodes initSock() = 0;

        /**
         * @brief Function called by the destructor to close the sockets
         * @note Override for it to be called by destructor
         */
        virtual void quit() = 0;

    private:
        /******************************************** Private Variables ********************************************/

        mutable bool    should_exit;        // true if should exit/stop connection


}; // end of TcpClient class


} // end of Network namespace

#endif
