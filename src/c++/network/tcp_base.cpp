#include "tcp_base.h"

namespace RPI {
namespace Network {

using std::cout;
using std::cerr;
using std::endl;


/********************************************** Constructors **********************************************/

TcpBase::TcpBase()
    : Packet{}
    , should_exit{false}
    , net_agent_thread{}
    , started_threads{false}
    , has_cleaned_up{false}
{
    // stub
}

TcpBase::~TcpBase() {
    // stub
}

ReturnCodes TcpBase::cleanup() {
    // dont double cleanup
    if (has_cleaned_up) return ReturnCodes::Success;

    // wait to block until a thread has been setup
    // otherwise thread is empty and joins immediately
    // but max out time to wait or else program gets blocked here if thread is never started
    std::unique_lock<std::mutex> lk{thread_mutex};
    thread_cv.wait_for(
        lk,
        std::chrono::milliseconds(200),
        [&](){ return started_threads.load(); }
    );

    // block until thread ends
    if (net_agent_thread.joinable()) {
        net_agent_thread.join();
    }

    if(cam_vid_thread.joinable()) {
        cam_vid_thread.join();
    }

    // call quit once thread for derived client/server is over
    // quit should be overriden by derived classes for proper cleanup
    quit();

    has_cleaned_up = true;
    return ReturnCodes::Success;
}

/********************************************* Getters/Setters *********************************************/

ReturnCodes TcpBase::setExitCode(const bool new_exit) {
    should_exit.store(new_exit);
    return ReturnCodes::Success;
}

bool TcpBase::getExitCode() const {
    return should_exit.load();
}

void TcpBase::setRecvCallback(const RecvPktCallback& recv_callback) {
    recv_cb = recv_callback;
}

/****************************************** Shared Common Functions ****************************************/

int TcpBase::CloseOpenSock(int sock_fd) {
    if(sock_fd >= 0) {
        close(sock_fd);
        sock_fd = - 1;
    }
    return sock_fd;
}


void TcpBase::runNetAgent(const bool print_data) {
    // create a lock that prevents joiner from trying to join() before ready
    // if thread is not initialized yet, join() will occur before thread is started
    std::unique_lock<std::mutex> start_thread_lk{thread_mutex};

    // startup client/server agent in a thread
    // cannot capture by reference in locally existing lambda
    net_agent_thread = std::thread{[this, print_data]() mutable {
        // dont pin fn to TcpBase since it should be overridden by derived classes
        netAgentFn(print_data);
    }};

    cam_vid_thread = std::thread{[this]() {
        // dont pin fn to TcpBase since it should be overridden by derived classes
        VideoStreamHandler();
    }};

    // unlock & notify so joiner can continue
    started_threads.store(true);
    start_thread_lk.unlock();
    thread_cv.notify_all();
    
}

std::string TcpBase::formatIpAddr(const std::string& ip, const int port) const {
    return {ip + ":" + std::to_string(port)};
}

RecvRtn TcpBase::recvData(int socket_fd) {
    // make sure data socket is open/valid first
    if(socket_fd < 0) {
        return RecvRtn{{}, RecvRtnCodes::Error};
    }

    /*************************************** recv data pkt header *************************************/
    // first recv packet header to see how much data is expected (keep looping until it all arrives)
    const int header_size {sizeof(HeaderPkt_t)};
    char header_buf[header_size];
    std::size_t header_rx_size {0};
    while (header_rx_size < header_size) {
        const int header_rx_partial = ::recv(socket_fd, header_buf+header_rx_size, header_size, 0);
        if (header_rx_partial < 0) {
            cerr << "Error: receiving header packet" << endl;
            return RecvRtn{{}, RecvRtnCodes::Error};
        } 
        else if (header_rx_partial == 0) {
            cerr << "Error: other host closed connection while sending header packet" << endl;
            return RecvRtn{{}, RecvRtnCodes::ClosedConn};
        }
        header_rx_size =+ header_rx_partial;
        cout << header_rx_size << "/" << header_size << endl;
    }
    
    // reconstitute header packet into struct (in binary form)
    std::istringstream header_pkt_stream{ std::string{header_buf, header_rx_size} };
    // convert into actual struct
    HeaderPkt_t header { header_pkt_stream };
    cout << "header pkt (size: " << header_rx_size << "/" << header_size << ")" << endl;
    cout << "should recv " << header.total_length << endl;

    /*********************************** recv actual data packets *************************************/

    // prepare bufs for receiving
    std::uint32_t total_recv_size {0};
    char recv_buf[header.total_length];
    int count {0}; // REMOVE
    while (total_recv_size < header.total_length) {
        // append new data to top of buf (new start = start + curr size)
        const int rcv_size = ::recv(socket_fd, recv_buf+total_recv_size, Constants::Network::MAX_DATA_SIZE, 0);
        if(rcv_size < 0) {
            std::cout << "ERROR: Receive" << std::endl;
            break;
        }
        total_recv_size += rcv_size;
        
        // REMOVE
        count++;
        if (count % 20 == 0)
            cout << total_recv_size << "/" << header.total_length << endl;
    }

    return RecvRtn{
        std::vector<u_char>{recv_buf, recv_buf+header.total_length},
        total_recv_size > 0 ? 
            RecvRtnCodes::Sucess :
            total_recv_size == 0 ? RecvRtnCodes::ClosedConn : RecvRtnCodes::Error
    };
}

int TcpBase::sendData(
    int& socket_fd,
    const void* buf,
    const std::uint32_t size_to_tx,
    const bool ignore_broken_pipe
) {
    // make sure data socket is open/valid first
    if(socket_fd < 0) {
        return -1;
    }

    // setup some send flags
    const int send_flags { ignore_broken_pipe ? MSG_NOSIGNAL : 0};

    // construct header packet to send pkt to send prior to data
    HeaderPkt_t header_pkt      {};
    header_pkt.total_length     = size_to_tx;
    header_pkt.checksum         = header_pkt.CalcChecksum(buf, size_to_tx);
    cout << "Sending " << size_to_tx << endl;

    /*************************************** send data pkt header *************************************/
    // send the header for the packet so recv can know size (send with no special props)
    // const std::string str_pkt {header_pkt.toString()};
    std::string str_pkt {header_pkt.toString()};

    // todo remove after testing!!!
    auto test_stream {std::istringstream(str_pkt, std::ios_base::binary)};
    HeaderPkt_t test_pkt { test_stream };

    const int header_send_rtn = ::send(socket_fd, str_pkt.c_str(), str_pkt.size(), 0);
    if(header_send_rtn < 0) {
        cerr << "ERROR: Failed to send header packet!" << endl;
        return header_send_rtn;
    }

    /************************************** send actual data pkts *************************************/
    // send actual data now that other side knows size of this packet
    const int sent_size = ::send(socket_fd, buf, size_to_tx, send_flags);

    // error check (-1 in case of errors)
    if(sent_size < 0) {
        cout << "ERROR: Sending data packet" << std::endl;
    }
    return sent_size;
}


/********************************************* Helper Functions ********************************************/

// Credit: https://stackoverflow.com/a/3120382/13933174
std::string TcpBase::GetPublicIp() const {
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

    char ip_buf[16];
    const char* p = inet_ntop(AF_INET, &name.sin_addr, ip_buf, sizeof(ip_buf));
    assert(p);

    close(sock);

    return std::string(ip_buf);
}

} // end of Network namespace

}; // end of RPI namespace
