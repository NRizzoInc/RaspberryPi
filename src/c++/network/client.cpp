#include "client.h"

namespace RPI {
namespace Network {

using std::cout;
using std::cerr;
using std::endl;

/********************************************** Constructors **********************************************/

TcpClient::TcpClient(const std::string& ip_addr, const int port_num, const bool should_init)
    : TcpBase{}
    , client_sock_fd{-1}        // init to invalid
    , server_ip{ip_addr}        // ip address to try to reach server
    , server_port{port_num}     // port the client tries to reach the server at
    , is_first_msg{true}        // will be set false immediately after sending first message
{
    // first check if should not init
    if (!should_init) return;

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


ReturnCodes TcpClient::updatePkt(const CommonPkt& updated_pkt) {
    // inform client that pkt has been updated and needs to be sent
    std::unique_lock<std::mutex> lk(data_mutex);
    auto rtn_code {Packet::updatePkt(updated_pkt)};
    lk.unlock();
    has_new_msg.notify_one();

    // wait for pkt to be sent by worker
    has_new_msg.wait(lk);

    // can return now that packet was sent
    return rtn_code;
}

/********************************************* Client Functions ********************************************/

void TcpClient::netAgentFn(const bool print_data) {
    /********************************* Connect Setup  ********************************/
    // connect to server (if failed to connect, just stop)
    if(connectToServer() != ReturnCodes::Success) {
        cerr << "ERROR: Failed to connect to server @" << formatIpAddr(server_ip, server_port) << endl;
        return;
    } else {
        cout << "Success: Connect to server" << endl;
    }

    // loop to receive data and send data to server
    while(!getExitCode()) {
        // wait until there is a new message (or first message)
        // or until server is about to timeout
        std::unique_lock<std::mutex> data_lock(data_mutex);
        has_new_msg.wait_for(
            data_lock,
            std::chrono::seconds(Constants::Network::RECV_TIMEOUT-1),
            [&](){return is_first_msg.load();}
        );
        // prevent predicate from being triggered in future iterations
        is_first_msg.store(false);

        /********************************* Sending To Server ********************************/
        // client starts by sending data to other endpoint
        // on first transfer will be sending zeroed out struct
        // the client should be continuously updating the packet so it is ready to send
        const std::string   json_str    {writePkt(getCurrentPkt())};
        data_lock.unlock();             // unlock after leaving critical region
        const char*         send_pkt    {json_str.c_str()};
        const std::size_t   pkt_size    {json_str.size()};

        // print the stringified json if told to
        if (print_data) {
            cout << "Sending (" << pkt_size << "Bytes): " << send_pkt << endl;
        }

        // send the stringified json to the server
        if(sendData(client_sock_fd, send_pkt, pkt_size) < 0) {
            cout << "Terminate - the other endpoint has closed the socket" << endl;
            setExitCode(true);
            break;
        }

        // client does not need to receive from server (YET)
        // TODO: implement method to receive camera data from server

        // inform updatePkt function that packet has been sent
        has_new_msg.notify_one();
    }
    cout << "Exiting client" << endl;
}


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

    // set receive timeout so that runNetAgent loop can be stopped/killed
    // without timeout accept connection might be blocking until a connection forms
    struct timeval timeout;
    timeout.tv_sec = Constants::Network::ACPT_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(client_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

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

ReturnCodes TcpClient::connectToServer() {
    // init struct for address to connect to
    sockaddr_in server_addr {};
    server_addr.sin_family      = AF_INET;                      // address family is AF_INET (IPV4)
    server_addr.sin_addr.s_addr = inet_addr(server_ip.c_str()); // convert str ip to binary ip representation
    server_addr.sin_port        = htons(server_port);           // convert server_port to network number format

    if (connect(client_sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) { 
        if(client_sock_fd >= 0) {
            close(client_sock_fd);
        }
        return ReturnCodes::Error;
    }

    return ReturnCodes::Success;
}


/********************************************* Helper Functions ********************************************/


} // end of Network namespace

}; // end of RPI namespace
