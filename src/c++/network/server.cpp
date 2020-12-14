#include "server.h"

using std::cout;
using std::cerr;
using std::endl;

namespace Network {


/********************************************** Constructors **********************************************/

TcpServer::TcpServer(const int port_num)
    : Packet{}
    , listen_sock_fd{-1}            // init to invalid
    , data_sock_fd{-1}              // init to invalid
    , client_ip{}                   // empty string bc no client yet
    , listen_port{port_num}         // wait to accept connections at this port
    , should_exit{false}            // not ready to end connection yet
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


    // wrap accept call in loop (due to timeout) to allow for program to be killed
    // should stop looping when the connection has been made (i.e. data sock is positive)
    while (!should_exit && data_sock_fd < 0) {
        // call the accept API on the socket and forward connection to data socket
        cout << "Waiting to accept connection @" << formatIpAddr() << endl;
        data_sock_fd = ::accept(listen_sock_fd, (struct sockaddr*) &client_addr, &addr_l);
    }

    // if should exit, do not continue
    if (should_exit) {
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

    // set receive timeout so that runServer loop can be stopped/killed
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

int TcpServer::recvData(char* buf) {
    // make sure data socket is open/valid first
    if(data_sock_fd < 0) {
        return -1;
    }

    // call the recv API
    int rcv_size = ::recv(data_sock_fd, buf, Constants::Network::MAX_DATA_SIZE, 0);

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

void TcpServer::runServer(const bool print_data) {
    // create a char buffer called buf, of size C_MTU
    char buf[Constants::Network::MAX_DATA_SIZE];

    // wait for a client to connect
    if(acceptClient() == ReturnCodes::Success) {

        // loop to receive data and send application ACKs to this client
        while(!should_exit) {

            // call recvData, passing buf, to receive data
            // save the return value of recvData in a data_size variable
            const int data_size {recvData(buf)};

            // check if the data_size is smaller than 0
            // (if so, time to end loop & exit)
            if (data_size < 0) {
                cout << "Terminate - socket recv error" << endl;
                should_exit = true;
                break;
            }

            // check if the data_size is equal to 0 (time to exit)
            else if (data_size == 0) {
                cout << "Terminate - the other endpoint has closed the socket" << endl;
                should_exit = true;
                break;
            } 

            // print the buf to the terminal(if told to)
            const json json_pkt {readPkt(buf)};
            const CommonPkt pkt {interpretPkt(json_pkt)};
            if (print_data) {
                cout << json_pkt.dump() << endl;
            }

            // reset the buffer for a new read
            memset(buf, 0, sizeof(buf));

            // send an application ACK to the other endpoint
            // negative return == error
            // TODO: Remove return-to-sender duplicate
            const char* send_pkt {writePkt(pkt)};
            if(sendData(send_pkt, sizeof(send_pkt)) < 0) {
                should_exit = true;
                break;
            }
        }
    }
}


ReturnCodes TcpServer::optionsAndBind() {
    // set the options for the socket
    int option(1);
    setsockopt(listen_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));

    // set receive timeout so that runServer loop can be stopped/killed
    // without timeout accept connection might be blocking until a connection forms
    struct timeval timeout;
    timeout.tv_sec = Constants::Network::ACPT_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(listen_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

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

// Credit: https://stackoverflow.com/a/3120382/13933174
void TcpServer::GetPublicIp(char* buffer, std::size_t buf_size) const {
    assert(buf_size >= 16);

    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        cerr << "ERROR: Creating temp socket to get public ip" << endl;
    }

    const char* kGoogleDnsIp = "8.8.8.8";
    uint16_t kDnsPort = 53;
    struct sockaddr_in serv;
    memset(&serv, 0, sizeof(serv));
    serv.sin_family = AF_INET;
    serv.sin_addr.s_addr = inet_addr(kGoogleDnsIp);
    serv.sin_port = htons(kDnsPort);

    int err = connect(sock, (const sockaddr*) &serv, sizeof(serv));
    assert(err != -1);

    sockaddr_in name;
    socklen_t namelen = sizeof(name);
    err = getsockname(sock, (sockaddr*) &name, &namelen);
    assert(err != -1);

    const char* p = inet_ntop(AF_INET, &name.sin_addr, buffer, buf_size);
    assert(p);

    close(sock);
}

std::string TcpServer::formatIpAddr() const {
    char ip_buf[16];
    GetPublicIp(ip_buf, sizeof(ip_buf));
    return {std::string(ip_buf) + ":" + std::to_string(listen_port)};
}



} // end of Network namespace
