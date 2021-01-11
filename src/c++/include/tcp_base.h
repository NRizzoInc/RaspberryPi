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
#include <sstream> // for packet stream

// Our Includes
#include "constants.h"
#include "packet.h"

// 3rd Party Includes

namespace RPI {
namespace Network {

// holds the return of recvData(), check the "RtnCode" attribute to see if any errors occured
enum class RecvSendRtnCodes {
    Error,
    ClosedConn,
    Sucess
};
struct RecvRtn {
    std::vector<u_char> buf;   // the data receivied via the socket (data.size() for size)
    RecvSendRtnCodes    RtnCode;
};

struct SendRtn {
    std::uint32_t       size;   // the size of the  sent data -- pinned to uint32_t bc thats max send size currently
    RecvSendRtnCodes    RtnCode;
};


/**
 * @brief Implements common features shared between server & client
 * 
 */
class TcpBase : public Packet {
    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a TcpBase object (should be created via cast from a derived class)
         * @param verbosity If true, will print more information that is strictly necessary
         */
        TcpBase(const bool verbosity=false);
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
         * @note Starts up all network related threads
         * @param print_data Should received data be printed?
         */
        void runNetAgent(const bool print_data);

        
        bool getIsInit() const;
        void setIsInit(const bool new_status);

        /**
         * @brief Sends a reset packet to the other host
         * @return Success if no issues
         */
        virtual ReturnCodes sendResetPkt() = 0;

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
         * @return string containing the data - check its size/exists for number of bytes received:
         * (std::nullopt if error occurred)
         * (0  if closed connection)
         */
        virtual RecvRtn recvData(int socket_fd);

        /**
         * @brief Send data to remote host.
         * @param socket_fd The receiving socket's file descriptor
         * @param buf pointer to the buffer where the data to be sent is stored - can be (un)signed char
         * @param size_to_tx size to transmit
         * (other host closes conn) & instead returns EPIPE (negative)
         * @return number of bytes sent (RtnCode == Success if no issues)
         * - max size is std::uint32_t bc thats the max packet length
         * @todo Handle packet sizes larger that automatically loop when sending packets
         * @note sends will not result in a broken SIGPIPE signal to prevent program from being killed
         */
        virtual SendRtn sendData(
            int& socket_fd,
            const void* buf,
            const std::uint32_t size_to_tx
        );

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
        virtual void ControlLoopFn(const bool print_data) = 0;

        /**
         * @brief The function to run regarding video frames when starting up the TCP server/client
         * (override so that it can be called by runNetAgent() in a thread)
         */
        virtual void VideoStreamHandler() = 0;

        /**
         * @brief Function called by the destructor to close the sockets
         * @note This should be called at beginining of derived quit()
         */
        virtual void quit() = 0;

        /***************************** Protected Variables (Both Client/Server Can Use) *****************************/
    protected:
        RecvPktCallback             recv_cb;            // callback for when a packet is received

        /**
         * @brief Helper function that closes and sets a socket file descriptor to -1 if it is open
         * @param sock_fd The socket file descriptor to close if open
         * @return The socket file descriptors new value (-1)
         */
        virtual int CloseOpenSock(int sock_fd);

    private:
        /******************************************** Private Variables ********************************************/

        const bool                  is_verbose;         // false if should only print errors/important info
        std::atomic_bool            should_exit;        // true if should exit/stop connection
        std::thread                 control_thread;     // holds the thread proc for ControlLoopFn()
        std::thread                 cam_vid_thread;     // holds the thread proc for VideoStreamHandler()
        std::atomic_bool            started_threads;    // need to send an initization message for first packet
        std::mutex                  thread_mutex;       // mutex controlling access to the classes threads (start/join)
        std::condition_variable     thread_cv;          // true if client needs to tell the server something
        std::atomic_bool            is_init;            // helps determine if needs to cleanup in derived classes
        std::atomic_bool            has_cleaned_up;     // makes sure cleanup doesnt happen twice

}; // end of TcpClient class


} // end of Network namespace

}; // end of RPI namespace

#endif
