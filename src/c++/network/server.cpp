#include "server.h"

using std::cout;
using std::cerr;
using std::endl;

namespace RPI {
namespace Network {


/********************************************** Constructors **********************************************/

TcpServer::TcpServer(const int port_num, const bool should_init)
    : TcpBase{}
    , listen_sock_fd{-1}            // init to invalid
    , data_sock_fd{-1}              // init to invalid
    , client_ip{}                   // empty string bc no client yet
    , listen_port{port_num}         // wait to accept connections at this port
{
    // first check if should not init
    if (!should_init) return;

    // call the function to create socket, set the options and bind,
    // and close the socket and return if not successful
    if(initSock() != ReturnCodes::Success) {
        cout << "ERROR: Initializing Server Socket" << endl;
        quit();
        return;
    }
}

TcpServer::~TcpServer() {
    quit();
}


/********************************************* Getters/Setters *********************************************/


/********************************************* Server Functions ********************************************/

ReturnCodes TcpServer::acceptClient() {
    // prepare the struct to store the client address
    sockaddr_in client_addr;
    socklen_t addr_l = sizeof(client_addr);


    // wrap accept call in loop (due to timeout) to allow for program to be killed
    // should stop looping when the connection has been made (i.e. data sock is positive)
    cout << "Waiting to accept connection @" << formatIpAddr(GetPublicIp(), listen_port) << endl;
    while (!getExitCode() && data_sock_fd < 0) {
        // call the accept API on the socket and forward connection to data socket
        data_sock_fd = ::accept(listen_sock_fd, (struct sockaddr*) &client_addr, &addr_l);
    }

    // if should exit, do not continue
    if (getExitCode()) {
        return ReturnCodes::Error;
    }

    // if the data socket does not open successfully, close the listening socket
    if(data_sock_fd < 0) {
        cout << "ERROR: Failed to accept connect" << endl;
        if(data_sock_fd >= 0) {
            close(data_sock_fd);
            data_sock_fd = - 1;
        }
        return ReturnCodes::Error;
    }

    // set receive timeout so that runNetAgent loop can be stopped/killed
    // without timeout recv might be blocking and loop might not end
    struct timeval timeout;
    timeout.tv_sec = Constants::Network::RECV_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(data_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    // Print the client address (convert network address to char)
    cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << endl; 

    // save the client IP in the m_ip string
    client_ip = inet_ntoa(client_addr.sin_addr);
    return ReturnCodes::Success;
}

void TcpServer::netAgentFn(const bool print_data) {
    // create a char buffer that hold the max allowed size
    char buf[Constants::Network::MAX_DATA_SIZE];

    // loop to keep trying to connect to new clients until told to stop
    while(!getExitCode()) {

        // wait for a client to connect
        if(acceptClient() == ReturnCodes::Success) {

            // loop to receive data and send data with client
            while(!getExitCode()) {

                /********************************* Receiving From Server ********************************/
                // call recvData, passing buf, to receive data
                // save the return value of recvData in a data_size variable
                const int data_size {recvData(data_sock_fd, buf)};

                // check if the data_size is smaller than 0
                // (if so, print message bc might have been fluke)
                if (data_size < 0) {
                    cout << "Terminate - socket recv error" << endl;
                }

                // check if the data_size is equal to 0 (time to exit bc client killed conn)
                // break, but dont exit so server can wait for new client to connect
                else if (data_size == 0) {
                    cout << "Terminate - the other endpoint has closed the socket" << endl;
                    break;
                } 

                // print the buf to the terminal(if told to)
                if (print_data) {
                    cout << "Recv: " << buf << endl;
                }

                // convert stringified json to json so it can be parsed into struct
                try {
                    const CommonPkt pkt {readPkt(buf)};
                    if(updatePkt(pkt) != ReturnCodes::Success) {
                        cerr << "Failed to update from client info" << endl;
                    }

                    // call receive callback if set
                    if (recv_cb) {
                        if (recv_cb(pkt) != ReturnCodes::Success) {
                            cerr << "ERROR: Failed to process received packet from client" << endl;
                        }
                    }
                } catch (std::exception& err) {
                    cerr << "Failed to update from client info" << endl;
                    cerr << err.what() << endl;
                }

                // reset the buffer for a new read
                memset(buf, 0, sizeof(buf));

                // server does not need to send data to client (YET)
                // TODO: implement method to send camera data to client
            }

            // at end of while, reset data socket to attempt to make new connection with same listener
            if (data_sock_fd > 0) {
                close(data_sock_fd);
                data_sock_fd = -1;
            }
        }
    }
}

ReturnCodes TcpServer::initSock() {
    // open the listen socket of type SOCK_STREAM (TCP)
    listen_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // check if the socket creation was successful
    if (listen_sock_fd < 0){ 
        cout << "ERROR: Opening Listen Socket" << endl;
        return ReturnCodes::Error;
    }

    // set the options for the socket
    int option(1);
    setsockopt(listen_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));

    // set receive timeout so that runNetAgent loop can be stopped/killed
    // without timeout accept connection might be blocking until a connection forms
    struct timeval timeout;
    timeout.tv_sec = Constants::Network::ACPT_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(listen_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    // init struct for address to bind socket
    sockaddr_in my_addr {};
    my_addr.sin_family = AF_INET;                   // address family is AF_INET (IPV4)
    my_addr.sin_port = htons(listen_port);          // convert listen_port to network number format
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // accept conn from all Network Interface Cards (NIC)

    // bind the socket to the port
    if (bind(listen_sock_fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0) { 
        cout << "ERROR: Failed to bind socket" << endl;
        if(listen_sock_fd>=0) {
            close(listen_sock_fd);
        }
        return ReturnCodes::Error;
    }

    // accept at most 1 client at a time, and set the socket in a listening state
    if (listen(listen_sock_fd, 1) < 0) { 
        cout << "ERROR: Failed to listen to socket" << endl;
        close(listen_sock_fd);
        return ReturnCodes::Error;
    }
    return ReturnCodes::Success;
}

void TcpServer::quit() {
    // set exit status to be true
    setExitCode(true);

    // if listen socket is open, close it and set to -1
    if (listen_sock_fd > 0) {
        close(listen_sock_fd);
        listen_sock_fd = -1;
    }

    if (data_sock_fd > 0) {
        close(data_sock_fd);
        data_sock_fd = -1;
    }
}


/********************************************* Helper Functions ********************************************/


} // end of Network namespace

}; // end of RPI namespace
