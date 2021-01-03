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

int TcpBase::recvData(
    int socket_fd,
    char* buf,
    const std::size_t max_buf_size,
    const bool wait_for_all
) {
    // make sure data socket is open/valid first
    if(socket_fd < 0) {
        return -1;
    }

    // call the recv API
    cout << "receiving max data size: " << max_buf_size << endl;
    const int recv_flags { wait_for_all ? MSG_WAITALL : 0 };
    int rcv_size = ::recv(socket_fd, buf, max_buf_size, recv_flags);

    /*
    if(rcv_size < 0) {
        std::cout << "ERROR: Receive" << std::endl;

        // close just the data socket bc done receiving from client
        // but want to still listen for new connections
        if(socket_fd >= 0) {
            ::close(socket_fd);
            socket_fd = -1;
        }
    }
    */

    return rcv_size;
}

int TcpBase::sendData(int& socket_fd, const char* buf, const size_t size_to_tx) {
    // make sure data socket is open/valid first
    if(socket_fd < 0) {
        return -1;
    }

    // send the data through sckfd
    const int sent_size = ::send(socket_fd, buf, size_to_tx, 0);

    // error check (-1 in case of errors)
    // if error close the socket and exit
    if(sent_size < 0) {
        std::cout << "ERROR: Send" << std::endl;
        if(socket_fd >= 0) {
            close(socket_fd);
            socket_fd = -1;
        }
        return -4;
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
