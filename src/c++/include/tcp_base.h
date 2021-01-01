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
#include <atomic> // for exit variable
#include <thread> // to store thread object
// block destructor with mutex so that thread can be created prior to joining
#include <atomic>
#include <mutex>
#include <condition_variable>

// Our Includes
#include "constants.h"
#include "packet.h"

// 3rd Party Includes

namespace RPI {
namespace Network {

/**
 * @brief Implements common features shared between server & client
 * 
 */
class TcpBase : public Packet {
    public:
        /********************************************** Constructors **********************************************/

        TcpBase();
        virtual ~TcpBase();

        /**
         * @brief Wrapper for wrapping and closing functions that need to be called
         * before end object goes out of scope. (calls quit() & other required functions)
         * @return ReturnCodes 
         */
        virtual ReturnCodes cleanup();

        /********************************************* Getters/Setters *********************************************/

        /**
         * @brief Set the callback function for when a packet is received
         * @param recv_callback The function that accepts a reference to the received pkt
         * @returns ReturnCodes::Success for no issues or ReturnCodes::Error if there was a problem
         * @return ReturnCodes 
         */
        void setRecvCallback(const RecvPktCallback& recv_callback);

        /**
         * @brief Sets the exit code. 
         * @param new_exit true TcpServer is should exit
         * @param new_exit false TcpServer should still run and not ready to exit
         * @note Useful for terminating runNetAgent() from main thread
         */
        ReturnCodes setExitCode(const bool new_exit);

        /**
         * @brief Get the current exit code status
         * @return true TcpServer is ready to or should exit
         * @return false TcpServer is still running and not ready to exit
         */
        bool getExitCode() const;

        /**
         * @brief Starts up a non-blocking read for the server/client
         * @note Calls netAgentFn() in a thread
         * @param print_data Should received data be printed?
         */
        void runNetAgent(const bool print_data);

    protected:
        /****************************************** Shared Common Functions ****************************************/

        /**
         * @brief Get the public ip address of the device
         * @return A string containing the public ip address
         * @credit: https://stackoverflow.com/a/3120382/13933174
         */
        std::string GetPublicIp() const;

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
         * @brief The function to run when starting up the TCP server/client
         * (override so that it can be called by runNetAgent() in a thread)
         * @param print_data Should received data be printed?
         */
        virtual void netAgentFn(const bool print_data) = 0;

        /**
         * @brief Function called by the destructor to close the sockets
         * @note This should be called at beginining of derived quit()
         */
        virtual void quit() = 0;

        /***************************** Protected Variables (Both Client/Server Can Use) *****************************/
    protected:
        RecvPktCallback             recv_cb;            // callback for when a packet is received

    private:
        /******************************************** Private Variables ********************************************/

        std::atomic_bool            should_exit;        // true if should exit/stop connection
        std::thread                 net_agent_thread;   // holds the thread proc for runNetAgent()
        std::atomic_bool            started_thread;     // need to send an initization message for first packet
        std::mutex                  thread_mutex;
        std::condition_variable     thread_cv;          // true if client needs to tell the server something
        bool                        has_cleaned_up;     // makes sure cleanup doesnt happen twice

}; // end of TcpClient class


} // end of Network namespace

}; // end of RPI namespace

#endif
