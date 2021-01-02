#include "tcp_server.h"

using std::cout;
using std::cerr;
using std::endl;

namespace RPI {
namespace Network {


/********************************************** Constructors **********************************************/

TcpServer::TcpServer(const int ctrl_data_port, const int cam_send_port, const bool should_init)
    : TcpBase{}
    , ctrl_listen_sock_fd{-1}               // init to invalid
    , ctrl_data_sock_fd{-1}                 // init to invalid
    , client_ip{}                           // empty string bc no client yet
    , ctrl_data_port{ctrl_data_port}        // wait to accept connections at this port for regular pkts
    , cam_listen_sock_fd{-1}                // init to invalid
    , cam_data_sock_fd{-1}                  // init to invalid
    , cam_data_port{cam_send_port}          // port for the camera data connection
    , has_new_cam_data{true}                // will be set false immediately after sending first message
{
    // first check if should not init
    if (!should_init) return;

    // call the function to create socket, set the options and bind,
    // and close the socket and return if not successful
    if(initSock() != ReturnCodes::Success) {
        cout << "ERROR: Initializing Server Sockets" << endl;
        quit();
        return;
    }
}

TcpServer::~TcpServer() {
    quit();
}


/********************************************* Getters/Setters *********************************************/


/********************************************* Server Functions ********************************************/

ReturnCodes TcpServer::acceptClient(int& data_sock_fd, const std::string& conn_desc, const int port) {
    // prepare the struct to store the client address
    sockaddr_in client_addr;
    socklen_t client_addr_l = sizeof(client_addr);


    // wrap accept call in loop (due to timeout) to allow for program to be killed
    // should stop looping when the connection has been made (i.e. data sock is positive)
    cout << "Waiting to accept " << conn_desc << " data connection @" << formatIpAddr(GetPublicIp(), port) << endl;
    while (!getExitCode() && (ctrl_data_sock_fd < 0 || cam_data_sock_fd < 0)) {
        // call the accept API on the socket and forward connection to data socket
        data_sock_fd = ::accept(data_sock_fd, (struct sockaddr*) &client_addr, &client_addr_l);
    }

    // if should exit, do not continue
    if (getExitCode()) {
        return ReturnCodes::Error;
    }

    // if the data socket does not open successfully, close the listening socket
    if(ctrl_data_sock_fd < 0 || cam_data_sock_fd < 0) {
        cout << "ERROR: Failed to accept " << conn_desc << " connection" << endl;
        data_sock_fd = CloseOpenSock(data_sock_fd);
        return ReturnCodes::Error;
    }

    // set receive timeout so that runNetAgent loop can be stopped/killed
    // without timeout recv might be blocking and loop might not end
    struct timeval timeout;
    timeout.tv_sec = Constants::Network::RECV_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(data_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    // Print the client address (convert network address to char)
    cout << "New " << conn_desc << " connection from " << inet_ntoa(client_addr.sin_addr) << endl; 

    // save the client IP in the m_ip string
    client_ip = inet_ntoa(client_addr.sin_addr);
    return ReturnCodes::Success;
}

ReturnCodes TcpServer::setLatestCamFrame(const std::vector<char>& new_frame) {
    Packet::setLatestCamFrame(new_frame);
    has_new_cam_data.store(true);
    return ReturnCodes::Success;
}

const std::vector<char>& TcpServer::getLatestCamFrame() const {
    has_new_cam_data.store(false);
    return Packet::getLatestCamFrame();
}

void TcpServer::netAgentFn(const bool print_data) {
    // create a char buffer that hold the max allowed size
    char buf[Constants::Network::MAX_DATA_SIZE];

    // loop to keep trying to connect to new clients until told to stop
    while(!getExitCode()) {

        // wait for a client to connect
        if(acceptClient(ctrl_data_sock_fd, "control", ctrl_data_port) == ReturnCodes::Success) {

            // loop to receive data and send data with client
            while(!getExitCode()) {

                /********************************* Receiving From Server ********************************/
                // call recvData, passing buf, to receive data
                // save the return value of recvData in a data_size variable
                const int data_size {recvData(ctrl_data_sock_fd, buf)};

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
            }

            // at end of while, reset data socket to attempt to make new connection with same listener
            ctrl_data_sock_fd = CloseOpenSock(ctrl_data_sock_fd);
        }
    }
}

void TcpServer::VideoStreamHandler() {
    // server has to send the most up to date video frame to the client
    // keep sending until told to stop

    // loop to keep trying to connect to new clients until told to stop
    while(!getExitCode()) {

        // wait for a client to connect
        // note: extra space on desc string to even out prints
        if(acceptClient(cam_data_sock_fd, "camera ", cam_data_port) == ReturnCodes::Success) {

            // loop to receive data and send data with client
            while(!getExitCode()) {
            
                // wait until there is a new message (or first message)
                // or until server is about to timeout
                std::unique_lock<std::mutex> data_lock(cam_data_mutex);
                cam_data_cv.wait_for(
                    data_lock,
                    std::chrono::seconds(Constants::Network::RECV_TIMEOUT-1),
                    [&](){return has_new_cam_data.load();}
                );

                /********************************* Sending Camera Data to Client ********************************/
                const std::vector<char> cam_frame {getLatestCamFrame()};
                data_lock.unlock();             // unlock after leaving critical region

                // send the stringified json to the server
                if(sendData(cam_data_sock_fd, cam_frame.data(), cam_frame.size()) < 0) {
                    cout << "Terminate - the other endpoint has closed the socket" << endl;
                    setExitCode(true);
                    break;
                }

                // client does not need to receive from server (YET)
                // TODO: implement method to receive camera data from server

                // inform updatePkt function that camera packet has been sent
                cam_data_cv.notify_one();
            }

            // at end of while, reset data socket to attempt to make new connection with same listener
            cam_data_sock_fd = CloseOpenSock(cam_data_sock_fd);
        }
    }
}

ReturnCodes TcpServer::initSock() {
    // open the listen socket of type SOCK_STREAM (TCP)
    ctrl_listen_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    cam_listen_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // check if the socket creation was successful
    if (ctrl_listen_sock_fd < 0){ 
        cout << "ERROR: Opening control listen socket" << endl;
        return ReturnCodes::Error;
    }
    if (cam_listen_sock_fd < 0){ 
        cout << "ERROR: Opening camera listen socket" << endl;
        return ReturnCodes::Error;
    }


    // set the options for the socket
    int option(1);
    setsockopt(ctrl_listen_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));
    setsockopt(cam_listen_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));

    // set receive timeout so that runNetAgent loop can be stopped/killed
    // without timeout accept connection might be blocking until a connection forms
    struct timeval timeout;
    timeout.tv_sec = Constants::Network::ACPT_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(ctrl_listen_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(cam_listen_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    // init struct for address to bind socket
    sockaddr_in ctrl_addr {};   // holds data on the control conn address
    sockaddr_in cam_addr  {};   // holds data on the camera  conn address
    
    // address family is AF_INET (IPV4)
    ctrl_addr.sin_family        = AF_INET;
    cam_addr.sin_family         = AF_INET;
    // convert ctrl_data_port to network number format
    ctrl_addr.sin_port          = htons(ctrl_data_port);
    cam_addr.sin_port           = htons(cam_data_port);
    // accept conn from all Network Interface Cards (NIC)
    ctrl_addr.sin_addr.s_addr   = htonl(INADDR_ANY);
    cam_addr.sin_addr.s_addr    = htonl(INADDR_ANY);

    // bind the socket to the port
    if (bind(ctrl_listen_sock_fd, (struct sockaddr*)&ctrl_addr, sizeof(ctrl_addr)) < 0) { 
        cout << "ERROR: Failed to bind control socket" << endl;
        ctrl_listen_sock_fd = CloseOpenSock(ctrl_listen_sock_fd);
        return ReturnCodes::Error;
    }
    if (bind(cam_listen_sock_fd, (struct sockaddr*)&cam_addr, sizeof(cam_addr)) < 0) { 
        cout << "ERROR: Failed to bind camera socket" << endl;
        cam_listen_sock_fd = CloseOpenSock(cam_listen_sock_fd);
        return ReturnCodes::Error;
    }

    // accept at most 1 client at a time, and set the socket in a listening state
    const int max_num_conns {1};
    if (listen(ctrl_listen_sock_fd, max_num_conns) < 0) { 
        cout << "ERROR: Failed to listen to control socket" << endl;
        close(ctrl_listen_sock_fd);
        return ReturnCodes::Error;
    }
    if (listen(cam_listen_sock_fd, max_num_conns) < 0) { 
        cout << "ERROR: Failed to listen to control socket" << endl;
        close(cam_listen_sock_fd);
        return ReturnCodes::Error;
    }

    return ReturnCodes::Success;
}

void TcpServer::quit() {
    // set exit status to be true
    setExitCode(true);

    // if listen socket is open, close it and set to -1
    ctrl_listen_sock_fd = CloseOpenSock(ctrl_listen_sock_fd);
    ctrl_data_sock_fd = CloseOpenSock(ctrl_data_sock_fd);
}


/********************************************* Helper Functions ********************************************/


} // end of Network namespace

}; // end of RPI namespace
