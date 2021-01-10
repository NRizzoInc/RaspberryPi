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
    , control_thread{}
    , started_threads{false}
    , is_init{false}
    , has_cleaned_up{false}
{
    // stub
}

TcpBase::~TcpBase() {
    // stub
}

ReturnCodes TcpBase::cleanup() {
    // dont double cleanup
    if (has_cleaned_up.load()) return ReturnCodes::Success;

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
    if (control_thread.joinable()) {
        control_thread.join();
    }

    if(cam_vid_thread.joinable()) {
        cam_vid_thread.join();
    }

    // call quit once thread for derived client/server is over
    // quit should be overriden by derived classes for proper cleanup
    quit();

    has_cleaned_up.store(true);
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

bool TcpBase::getIsInit() const {
    return is_init.load();
}

void TcpBase::setIsInit(const bool new_status) {
    is_init.store(new_status);
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
    control_thread = std::thread{[this, print_data]() mutable {
        // dont pin fn to TcpBase since it should be overridden by derived classes
        ControlLoopFn(print_data);
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
        return RecvRtn{{}, RecvSendRtnCodes::Error};
    }

    /*************************************** recv data pkt header *************************************/
    // first recv packet header to see how much data is expected (keep looping until it all arrives)
    const int max_header_size {sizeof(HeaderPkt_t)};
    char header_buf[max_header_size];
    int header_rx_size {0};
    while (header_rx_size < max_header_size) {
        const int max_stream_left { std::min(
            max_header_size-header_rx_size, // amount of header pkt left to recv
            static_cast<int>(Constants::Network::MAX_DATA_SIZE)
        )};

        const int header_rx_partial = ::recv(socket_fd, header_buf+header_rx_size, max_stream_left, 0);
        if (header_rx_partial < 0) {
            // TODO: print this if --verbose
            // cerr << "Error: receiving header packet" << endl;
            return RecvRtn{{}, RecvSendRtnCodes::Error};
        }
        else if (header_rx_partial == 0) {
            // end of stream
            // TODO: print this if --verbose
            // cerr << "Error: other host closed connection while sending header packet" << endl;
            return RecvRtn{{}, RecvSendRtnCodes::ClosedConn};
        }
        header_rx_size += header_rx_partial;
    }
    
    // reconstitute header packet into struct (in binary stream form)
    std::istringstream header_pkt_stream{ std::string{header_buf, static_cast<std::size_t>(header_rx_size)} };
    // convert into actual struct
    HeaderPkt_t header { header_pkt_stream };

    /*********************************** recv actual data packets *************************************/

    // prepare bufs for receiving
    std::uint64_t total_recv_size {0};
    char recv_buf[header.total_length];
    RecvSendRtnCodes rtn_code {RecvSendRtnCodes::Sucess}; // default to success
    while (total_recv_size < header.total_length) {
        // append new data to top of buf (new start = start + curr size)
        const std::uint64_t max_stream_size {std::min(
            header.total_length-total_recv_size, // the amount of stream data left to receive
            static_cast<std::uint64_t>(Constants::Network::MAX_DATA_SIZE)
        )};
        const int rcv_size = ::recv(socket_fd, recv_buf+total_recv_size, max_stream_size, 0);
        if(rcv_size < 0) {
            // TODO: print this if --verbose
            cerr << "ERROR: RECV (" << total_recv_size << "/" << header.total_length << ")" << endl;
            rtn_code = RecvSendRtnCodes::Error;
            break;
        } else if (rcv_size == 0) {
            // TODO: print this if --verbose
            cerr << "ERROR: RECV - other host closed connection" << endl;
            rtn_code = RecvSendRtnCodes::ClosedConn;
            break;
        }
        total_recv_size += rcv_size;
    }

    return RecvRtn{
        std::vector<u_char>{recv_buf, recv_buf+total_recv_size},
        rtn_code
    };
}

SendRtn TcpBase::sendData(
    int& socket_fd,
    const void* buf,
    const std::uint32_t size_to_tx
) {
    // make sure data socket is open/valid first
    if(socket_fd < 0) {
        return SendRtn{0, RecvSendRtnCodes::Error};
    }

    // construct header packet to send pkt to send prior to data
    HeaderPkt_t header_pkt      {};
    header_pkt.total_length     = size_to_tx;
    header_pkt.checksum         = header_pkt.CalcChecksum(buf, size_to_tx);

    /*************************************** send data pkt header *************************************/
    // send the header for the packet so recv can know size (send with no special props)
    // const std::string str_pkt {header_pkt.toString()};
    std::string str_pkt {header_pkt.toString()};

    const int max_header_size {sizeof(HeaderPkt_t)};
    const int header_sent_size = ::send(socket_fd, str_pkt.c_str(), max_header_size, MSG_NOSIGNAL);
    if(header_sent_size < 0) {
        // TODO: print this if --verbose
        cerr << "ERROR: Send header packet (" << header_sent_size << "/" << max_header_size << ")" << endl;
        return SendRtn{0, RecvSendRtnCodes::Error};
    }


    /************************************** send actual data pkts *************************************/
    // send actual data now that other side knows size of this packet
    const int sent_size = ::send(socket_fd, buf, size_to_tx, MSG_NOSIGNAL);
    if (sent_size < 0 || sent_size != static_cast<int>(size_to_tx)) {
        // TODO: print this if --verbose
        cerr << "ERROR: Sending Data (" << sent_size << "/" << size_to_tx << ")" << endl;
        return SendRtn{static_cast<std::uint32_t>(sent_size), RecvSendRtnCodes::Error};
    }

    // iff successful, sent_size >= 0 so can safely cast to uint
    return SendRtn{static_cast<std::uint32_t>(sent_size), RecvSendRtnCodes::Sucess};
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
