#include "client.h"

namespace Network {

using std::cout;
using std::cerr;
using std::endl;

/********************************************** Constructors **********************************************/

TcpClient::TcpClient(const std::string& ip_addr, const int port_num)
    : NetCommon{}
    , client_sock_fd{-1}        // init to invalid
    , server_ip{ip_addr}        // ip address to try to reach server
    , server_port{port_num}     // port the client tries to reach the server at
{
    // call the function to create socket, set the options and bind,
    // and close the socket and return if not successful
    if(initSock() != ReturnCodes::Success) {
        cout << "ERROR: Initializing Client Socket" << endl;
        quit();
        return;
    }
}

TcpClient::~TcpClient() {
    quit();
}

/********************************************* Getters/Setters *********************************************/


/********************************************* Client Functions ********************************************/

ReturnCodes TcpClient::initSock() {
    // open the listen socket of type SOCK_STREAM (TCP)
    client_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // check if the socket creation was successful
    if (client_sock_fd < 0){ 
        cout << "ERROR: Opening Client Socket" << endl;
        return ReturnCodes::Error;
    }

    // set the options for the socket
    int option(1);
    setsockopt(client_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));

    // set receive timeout so that runServer loop can be stopped/killed
    // without timeout accept connection might be blocking until a connection forms
    struct timeval timeout;
    timeout.tv_sec = Constants::Network::ACPT_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(client_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    // init struct for address to connect to
    sockaddr_in server_addr {};
    server_addr.sin_family      = AF_INET;                      // address family is AF_INET (IPV4)
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str()); // convert str ip to binary ip representation
    server_addr.sin_port        = htons(server_port);           // convert server_port to network number format

    // connect to server
    if (connect(client_sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { 
        cout << "ERROR: Failed to connect to server" << endl;
        if(client_sock_fd >= 0) {
            close(client_sock_fd);
        }
        return ReturnCodes::Error;
    }

    return ReturnCodes::Success;
}

void TcpClient::quit() {
    // set exit status to be true
    setExitCode(true);

    // if client socket is open, close it and set to -1
    if(client_sock_fd >= 0) {
        close(client_sock_fd);
        client_sock_fd = -1;
    }
}


/********************************************* Helper Functions ********************************************/


} // end of Network namespace