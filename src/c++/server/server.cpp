#include "server.h"

using std::cout;
using std::cerr;
using std::endl;

namespace RPI {


/********************************************** Constructors **********************************************/

TcpServer::TcpServer(const int port_num)
    : listen_sock_fd{-1}            // init to invalid
    , data_sock_fd{-1}              // init to invalid
    , client_ip{}                     // empty string bc no client yet
    , listen_port{port_num}         // wait to accept connections at this port
    , should_exit{false}               // not ready to end connection yet
{
    // open the listen socket of type SOCK_STREAM (TCP)
    listen_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // check if the socket creation was successful
    if (listen_sock_fd < 0){ 
        cout << "ERROR: Opening Listen Socket" << endl;
        return;
    }

    // call the function to set the options and bind,
    // and close the socket and return if not successful
    if(optionsAndBind() != ReturnCodes::Success) {
        cout << "ERROR: Binding Listen Socket" << endl;
        close(listen_sock_fd);
        return;
    }
}

TcpServer::~TcpServer() {
    quit();
}


/********************************************* Getters/Setters *********************************************/

ReturnCodes TcpServer::setExitCode(const bool new_exit) const {
    should_exit = new_exit;
    return ReturnCodes::Success;
}

bool TcpServer::getExitCode() const {
    return should_exit;
}


/********************************************* Server Functions ********************************************/

ReturnCodes TcpServer::acceptClient() {
    // prepare the struct to store the client address
    sockaddr_in client_addr;
    socklen_t addr_l = sizeof(client_addr);

    // call the accept API on the socket and forward connection to data socket
    data_sock_fd = ::accept(listen_sock_fd, (struct sockaddr*) &client_addr, &addr_l);

    // if the data socket does not open successfully, close the listening socket
    if(data_sock_fd < 0) {
        cout << "ERROR: Failed to accept connect" << endl;
        if(data_sock_fd >= 0) {
            close(data_sock_fd);
            data_sock_fd = - 1;
        }
        return ReturnCodes::Error;
    }

    // Print the client address (convert network address to char)
    cout << "New connection from " << inet_ntoa(client_addr.sin_addr) << endl; 

    // save the client IP in the m_ip string
    client_ip = inet_ntoa(client_addr.sin_addr);
    return ReturnCodes::Success;
}

int TcpServer::recvData(char* buf) {
    // make sure data socket is open/valid first
    if(data_sock_fd < 0) {
        return -1;
    }

    // call the recv API
    int rcv_size = ::recv(data_sock_fd, buf, Constants::MAX_DATA_SIZE, 0);

    // check if the recv size is ok or not
    if(rcv_size < 0) {
        std::cout << "ERROR: Receive" << std::endl;

        // close just the data socket bc done receiving from client
        // but want to still listen for new connections
        if(data_sock_fd >= 0) {
            close(data_sock_fd);
            data_sock_fd = -1;
        }
    }
    return rcv_size;
}

int TcpServer::sendData(const char* buf, const size_t size_to_tx) {
    // make sure data socket is open/valid first
    if(data_sock_fd < 0) {
        return -1;
    }

    // send the data through sckfd
    const int sent_size = ::send(data_sock_fd, buf, size_to_tx, 0);

    // error check (-1 in case of errors)
    // if error close the socket and exit
    if(sent_size < 0) {
        std::cout << "ERROR: Send" << std::endl;
        if(data_sock_fd >= 0) {
            close(data_sock_fd);
            data_sock_fd = -1;
        }
        return -4;
    }
    return sent_size;
}


ReturnCodes TcpServer::optionsAndBind() {
    // set the options for the socket
    int option(1);
    setsockopt(listen_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));
    
    // init struct for address to bind socket
    sockaddr_in my_addr {};
    my_addr.sin_family = AF_INET;                   // address family is AF_INET (IPV4)
    my_addr.sin_port = htons(listen_port);          // convert m_port to network number format
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);    // accept conn from all Network Interface Cards (NIC)

    // bind the socket to the port
    if (bind(listen_sock_fd, (struct sockaddr*)&my_addr, sizeof(my_addr)) < 0) { 
        std::cout << "ERROR: Failed to bind socket" << std::endl;
        if(listen_sock_fd>=0) {
            close(listen_sock_fd);
        }
        return ReturnCodes::Error;
    }

    // accept at most 1 client at a time, and set the socket in a listening state
    if (listen(listen_sock_fd, 1) < 0) { 
        cout << "ERROR: Failed to listen to socket" << endl;
        
        return ReturnCodes::Error;
    }
    return ReturnCodes::Success;
}

void TcpServer::quit() {
    // set exit status to be true
    should_exit = true;

    // if listen socket is open, close it and set to -1
    if(listen_sock_fd >= 0) {
        close(listen_sock_fd);
        listen_sock_fd = -1;
    }

    // if data socket is open, close it and set it to -1
    if(data_sock_fd >= 0){
        close(data_sock_fd);
        data_sock_fd = -1;
    }

}


/********************************************* Helper Functions ********************************************/

} // end of RPI namespace
