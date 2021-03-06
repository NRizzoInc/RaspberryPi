#include "tcp_client.h"

namespace RPI {
namespace Network {

using std::cout;
using std::cerr;
using std::endl;

/********************************************** Constructors **********************************************/

TcpClient::TcpClient(
    const std::string& ip_addr,
    const int ctrl_port_num,
    const int cam_port_num,
    const int srv_data_port_num,
    const bool should_init,
    const bool verbosity
)
    : TcpBase{verbosity}
    , ctrl_data_sock_fd{-1}                 // init to invalid
    , server_ip{ip_addr}                    // ip address to try to reach server
    , ctrl_data_port{ctrl_port_num}         // port the client tries to reach the server at for sending control pkts
    , cam_data_sock_fd{-1}                  // init to invalid
    , cam_data_port{cam_port_num}           // port to attempt to connect to server to recv camera data
    , srv_data_sock_fd{-1}                  // init to invalid
    , srv_data_port{srv_data_port_num}      // port to attempt to connect to server to recv server data
{
    // first check if should not init
    if (!should_init) return;

    // call the function to create socket, set the options and bind,
    // and close the socket and return if not successful
    if(initSock() != ReturnCodes::Success) {
        cout << "ERROR: Initializing Client Sockets" << endl;
        quit();
        return;
    } else {
        setIsInit(true);
    }
}

TcpClient::~TcpClient() {
    // quit() called by TcpBase
}

/********************************************* Getters/Setters *********************************************/


/********************************************* Client Functions ********************************************/

ReturnCodes TcpClient::sendResetPkt() {
    // setting a new packet triggers control thread to send this new packet
    // special cases:
    //      Camera: on reset, camera should turn off
    CommonPkt reset_pkt {};
    reset_pkt.cntrl.camera.is_on = false;
    return updatePkt(reset_pkt);
}

void TcpClient::ControlLoopFn(const bool print_data) {
    /********************************* Connect Setup  ********************************/
    // connect to server (if failed to connect, just stop)
    if(connectToServer(ctrl_data_sock_fd, server_ip, ctrl_data_port, "control") != ReturnCodes::Success) {
        // if issue, return immediately to prevent further errors
        return;
    }

    // loop to receive data and send data to server
    while(!getExitCode()) {
        // wait until there is a new message (or first message)
        // or until server is about to timeout
        const int timeout_sec = Constants::Network::RX_TX_TIMEOUT-1;
        std::unique_lock<std::mutex> data_lock(cmn_data_pkt_mutex);
        has_new_cmn_data.wait_for(
            data_lock,
            timeout_sec > 0 ? std::chrono::seconds(timeout_sec) : std::chrono::milliseconds(500),
            [&](){return cmn_pkt_ready.load();}
        );

        // bc of class scope of cv, need to manually unlock
        // or else gets stuck when mutex needed in "getCurrentCmnPkt()"
        data_lock.unlock();

        // prevent predicate from being triggered in future iterations w/o being set by another thread
        cmn_pkt_ready.store(false);

        /********************************* Sending To Server ********************************/
        // client starts by sending data to other endpoint
        // on first transfer will be sending zeroed out struct
        // the client should be continuously updating the packet so it is ready to send
        const CommonPkt&    curr_pkt    {getCurrentCmnPkt()};
        const json&         pkt_json    {convertPktToJson(curr_pkt)};
        const std::string   bson_str    {writePkt(pkt_json)};
        const char*         send_pkt    {bson_str.c_str()};
        const std::size_t   pkt_size    {bson_str.size()};

        // print the stringified json if told to
        if (print_data) {
            cout << "Sending (" << pkt_size << "Bytes): " << pkt_json.dump() << endl;
        }

        // send the stringified json to the server
        const SendRtn send_rtn {sendData(ctrl_data_sock_fd, send_pkt, pkt_size)};
        if(send_rtn.RtnCode != RecvSendRtnCodes::Success) {
            cout << "Terminate - the server's control endpoint has closed the socket" << endl;
            setExitCode(true); // end program
            break;
        }

        // inform updatePkt function that packet has been sent
        has_new_cmn_data.notify_one();
    }
    cout << "Exiting Client Control Sender" << endl;
}

void TcpClient::VideoStreamHandler() {
    // connect to camera server (if failed to connect, just stop)
    // add space at end of "camera" to make prints even
    if(connectToServer(cam_data_sock_fd, server_ip, cam_data_port, "camera ") != ReturnCodes::Success) {
        // if issue, return immediately to prevent further errors
        return;
    }

    /********************************* Receiving From Server ********************************/
    while(!getExitCode()) {

        // recv image/frame in the form of a string container (to also store size)
        const RecvRtn img_recv { recvData(cam_data_sock_fd) };

        // check if the data_size is smaller than 0
        // (if so, print message bc might have been fluke)
        if (img_recv.RtnCode == RecvSendRtnCodes::Error) {
            cout << "Error: Failed to recv camera data" << endl;
            continue; // dont try to save a bad frame
        }

        // check if server killed conn
        // break, but dont exit so server can wait for new client to connect
        else if (img_recv.RtnCode == RecvSendRtnCodes::ClosedConn) {
            cout << "Terminate - the server's camera endpoint has closed the socket" << endl;
            setExitCode(true); // end program (make sure control socket also ends)
            break;
        }

        // if no issues, save the new video frame
        constexpr auto save_frame_err {"Failed to update camera data from server"};
        try {
            // convert string -> std::vector<unsigned char> by providing the start ptr and end
            if(setLatestCamFrame(img_recv.buf) != ReturnCodes::Success) {
                cerr << save_frame_err << endl;
            }
        } catch (std::exception& err) {
            cerr << save_frame_err << endl;
            cerr << err.what() << endl;
        }
    }

    // at end of while, reset data socket to attempt to make new connection with same listener
    cout << "Exiting Client Camera Receiver" << endl;
    cam_data_sock_fd = CloseOpenSock(cam_data_sock_fd);
}

void TcpClient::ServerDataHandler(const bool print_data) {
    // connect to server's listener trying to send out server data (if failed to connect, just stop)
    if(connectToServer(srv_data_sock_fd, server_ip, srv_data_port, "srv data") != ReturnCodes::Success) {
        // if issue, return immediately to prevent further errors
        return;
    }

    /********************************* Receiving From Server ********************************/
    while(!getExitCode()) {

        // recv image/frame in the form of a string container (to also store size)
        const RecvRtn       srv_data_recv { recvData(srv_data_sock_fd) };
        const std::string&  data          { std::string{srv_data_recv.buf.begin(), srv_data_recv.buf.end()} };

        // check if the data_size is smaller than 0
        // (if so, print message bc might have been fluke)
        if (srv_data_recv.RtnCode == RecvSendRtnCodes::Error) {
            cout << "Error: Failed to recv server data" << endl;
            continue; // dont try to save a bad frame
        }

        // check if server killed conn
        // break, but dont exit so server can wait for new client to connect
        else if (srv_data_recv.RtnCode == RecvSendRtnCodes::ClosedConn) {
            cout << "Terminate - the server data endpoint has closed the socket" << endl;
            setExitCode(true); // end program (make sure control socket also ends)
            break;
        }

        // if no issues, save the packet
        constexpr auto save_srv_data_err {"Failed to update server data pkt from server"};
        try {
                // note: data is transmitted as bson so have to interpret & parse pkt first
            const json& recv_json = readSrvPkt(data.c_str(), data.size());

            // parse & print the buf to the terminal(if told to)
            if (print_data) {
                cout << "Recv Server Data: " + recv_json.dump() << endl;
            }

            // actually try to parse recv packet into the struct
            const SrvDataPkt pkt {readSrvPkt(recv_json)};

            // actually update the saved most recent packet in memory
            if(updatePkt(pkt) != ReturnCodes::Success) {
                cerr << "Failed to update from server data pkt" << endl;
            }

        } catch (std::exception& err) {
            cerr << save_srv_data_err << endl;
            cerr << err.what() << endl;
        }
    }

    // at end of while, reset data socket to attempt to make new connection with same listener
    cout << "Exiting Client Server Data Receiver" << endl;
    srv_data_sock_fd = CloseOpenSock(srv_data_sock_fd);
}

ReturnCodes TcpClient::initSock() {
    // open the listen socket of type SOCK_STREAM (TCP)
    ctrl_data_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    cam_data_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    srv_data_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // check if the socket creation was successful
    if (ctrl_data_sock_fd < 0){ 
        cout << "ERROR: Opening Client Control Socket" << endl;
        return ReturnCodes::Error;
    }
    if (cam_data_sock_fd < 0){ 
        cout << "ERROR: Opening Client Camera Socket" << endl;
        return ReturnCodes::Error;
    }
    if (srv_data_sock_fd < 0){ 
        cout << "ERROR: Opening Client 'Server Data' Socket" << endl;
        return ReturnCodes::Error;
    }

    // set the options for the socket
    int option(1);
    setsockopt(ctrl_data_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));
    setsockopt(cam_data_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));
    setsockopt(srv_data_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));

    // set receive timeout so that runNetAgent loop can be stopped/killed
    // without timeout accept connection might be blocking until a connection forms
    struct timeval timeout;
    timeout.tv_sec = Constants::Network::ACPT_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(ctrl_data_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(cam_data_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(srv_data_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

    return ReturnCodes::Success;
}

void TcpClient::quit() {
    // set exit status to be true
    setExitCode(true);

    // dont bother continuing if not initialized
    if (!getIsInit()) return;

    // if client socket is open, close it and set to -1
    cout << "Cleanup: closing control sockets" << endl;
    ctrl_data_sock_fd = CloseOpenSock(ctrl_data_sock_fd);
    cout << "Cleanup: closing camera sockets" << endl;
    cam_data_sock_fd = CloseOpenSock(cam_data_sock_fd);
    cout << "Cleanup: closing server data sockets" << endl;
    srv_data_sock_fd = CloseOpenSock(srv_data_sock_fd);
}

ReturnCodes TcpClient::connectToServer(
    int& sock_fd,
    const std::string& ip,
    const int port,
    const std::string& conn_desc
) {
    sockaddr_in server_addr     {};                         // init struct for address to connect to
    server_addr.sin_family      = AF_INET;                  // address family is AF_INET (IPV4)
    server_addr.sin_addr.s_addr = inet_addr(ip.c_str());    // convert str ip to binary ip representation
    server_addr.sin_port        = htons(port);              // convert port to network number format

    // note, due to threading cout stream overlapping, couts should print a single concated string 
    if (connect(sock_fd, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        cerr << "ERROR: Failed to connect to server " + conn_desc + " @" + formatIpAddr(ip, port) + "\n";
        sock_fd = CloseOpenSock(sock_fd);
        return ReturnCodes::Error;
    } else {
        cout << "Success: Connected to server " + conn_desc + " stream @" + formatIpAddr(ip, port) + "\n";
    }

    return ReturnCodes::Success;
}


/********************************************* Helper Functions ********************************************/


} // end of Network namespace

}; // end of RPI namespace
