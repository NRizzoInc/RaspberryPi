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
    const bool should_init,
    const bool verbosity
)
    : TcpBase{verbosity}
    , ctrl_data_sock_fd{-1}                 // init to invalid
    , server_ip{ip_addr}                    // ip address to try to reach server
    , ctrl_data_port{ctrl_port_num}         // port the client tries to reach the server at for sending control pkts
    , pkt_ready{true}                       // will be set false immediately after sending first message
    , server_data_sock_fd{-1}               // init to invalid
    , server_data_port{cam_port_num}        // port to attempt to connect to client to send camera data
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


ReturnCodes TcpClient::updatePkt(const CommonPkt& updated_pkt) {
    // inform client that pkt has been updated and needs to be sent
    std::unique_lock<std::mutex> lk(data_mutex);
    ReturnCodes rtn_code = Packet::updatePkt(updated_pkt);
    lk.unlock();
    pkt_ready.store(true); // atomic should be done outside of lock
    has_new_msg.notify_all();

    // can return now that packet was sent
    return rtn_code;
}

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
        std::unique_lock<std::mutex> data_lock(data_mutex);
        has_new_msg.wait_for(
            data_lock,
            timeout_sec > 0 ? std::chrono::seconds(timeout_sec) : std::chrono::milliseconds(500),
            [&](){return pkt_ready.load();}
        );
        // prevent predicate from being triggered in future iterations w/o being set by another thread
        pkt_ready.store(false);

        /********************************* Sending To Server ********************************/
        // client starts by sending data to other endpoint
        // on first transfer will be sending zeroed out struct
        // the client should be continuously updating the packet so it is ready to send
        const CommonPkt&    curr_pkt    {getCurrCmnPkt()};
        data_lock.unlock();             // unlock after leaving critical region
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
        has_new_msg.notify_one();
    }
    cout << "Exiting Client Control Sender" << endl;
}

void TcpClient::ServerDataHandler(const bool print_data) {
    // connect to camera server (if failed to connect, just stop)
    // add space at end of "camera" to make prints even
    if(connectToServer(server_data_sock_fd, server_ip, server_data_port, "camera ") != ReturnCodes::Success) {
        // if issue, return immediately to prevent further errors
        return;
    }

    /********************************* Receiving From Server ********************************/
    while(!getExitCode()) {

        // call recvData and save result in str container to get size
        const RecvRtn      server_recv  { recvData(server_data_sock_fd) };
        const std::string& server_data  { std::string{server_recv.buf.begin(), server_recv.buf.end()} };

        // check if the data_size is smaller than 0
        // (if so, print message bc might have been fluke)
        if (server_recv.RtnCode == RecvSendRtnCodes::Error) {
            cout << "Error: Failed to recv server data" << endl;
            continue; // dont try to save a bad frame
        }

        /*
        // check if server killed conn
        // break, but dont exit so server can wait for new client to connect
        else if (img_recv.RtnCode == RecvSendRtnCodes::ClosedConn) {
            cout << "Terminate - the server's camera endpoint has closed the socket" << endl;
            setExitCode(true); // end program (make sure control socket also ends)
            break;
        }
        */

        // convert stringified json to json so it can be parsed into struct
        constexpr auto save_server_data_err {"Failed to update data from server"};
        try {
            // note: data is transmitted as bson so have to interpret & parse pkt first
            const json& recv_json = readServerPkt(server_data.c_str(), server_data.size());

            // parse & print the buf to the terminal(if told to)
            if (print_data) {
                // might contain non-ascii/non-utf8 chars from cam data
                // so handle errors by replacing img buffer with printable string
                json printable_json = recv_json;
                printable_json["cam"]["img"] = "[Image Buffer]";
                cout << "Recv Server Data: " + printable_json.dump() << endl;
            }

            // actually try to parse recv packet into the struct
            const ServerData_t& pkt {readServerPkt(recv_json)};

            // error check
            if(Packet::updatePkt(pkt) != ReturnCodes::Success) {
                cerr << save_server_data_err << endl;
            }

            if (recv_srv_cb) {
                if (recv_srv_cb(pkt) != ReturnCodes::Success) {
                    cerr << "ERROR: Failed to process received packet from server" << endl;
                }
            }

        } catch (std::exception& err) {
            cerr << save_server_data_err << endl;
            cerr << err.what() << endl;
        }
    }

    // at end of while, reset data socket to attempt to make new connection with same listener
    cout << "Exiting Client Camera Receiver" << endl;
    server_data_sock_fd = CloseOpenSock(server_data_sock_fd);
}


ReturnCodes TcpClient::initSock() {
    // open the listen socket of type SOCK_STREAM (TCP)
    ctrl_data_sock_fd = socket(AF_INET, SOCK_STREAM, 0);
    server_data_sock_fd = socket(AF_INET, SOCK_STREAM, 0);

    // check if the socket creation was successful
    if (ctrl_data_sock_fd < 0){ 
        cout << "ERROR: Opening Client Control Socket" << endl;
        return ReturnCodes::Error;
    }
    if (server_data_sock_fd < 0){ 
        cout << "ERROR: Opening Client Camera Socket" << endl;
        return ReturnCodes::Error;
    }

    // set the options for the socket
    int option(1);
    setsockopt(ctrl_data_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));
    setsockopt(server_data_sock_fd, SOL_SOCKET, SO_REUSEADDR, (char*)&option, sizeof(option));

    // set receive timeout so that runNetAgent loop can be stopped/killed
    // without timeout accept connection might be blocking until a connection forms
    struct timeval timeout;
    timeout.tv_sec = Constants::Network::ACPT_TIMEOUT;
    timeout.tv_usec = 0;
    setsockopt(ctrl_data_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));
    setsockopt(server_data_sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout, sizeof(timeout));

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
    server_data_sock_fd = CloseOpenSock(server_data_sock_fd);
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
