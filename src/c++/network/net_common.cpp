#include "net_common.h"

namespace Network {

using std::cout;
using std::cerr;
using std::endl;


/********************************************** Constructors **********************************************/

NetCommon::NetCommon()
    : Packet{}
    , should_exit{false}
{
    // stub
}

NetCommon::~NetCommon() {
    // stub
}

/********************************************* Getters/Setters *********************************************/

ReturnCodes NetCommon::setExitCode(const bool new_exit) const {
    should_exit = new_exit;
    return ReturnCodes::Success;
}

bool NetCommon::getExitCode() const {
    return should_exit;
}


/****************************************** Shared Common Functions ****************************************/

std::string NetCommon::formatIpAddr(const std::string& ip, const int port) const {
    return {ip + ":" + std::to_string(port)};
}

int NetCommon::recvData(int socket_fd, char* buf) {
    // make sure data socket is open/valid first
    if(socket_fd < 0) {
        return -1;
    }

    // call the recv API
    int rcv_size = ::recv(socket_fd, buf, Constants::Network::MAX_DATA_SIZE, 0);

    // check if the recv size is ok or not
    if(rcv_size < 0) {
        std::cout << "ERROR: Receive" << std::endl;

        // close just the data socket bc done receiving from client
        // but want to still listen for new connections
        if(socket_fd >= 0) {
            close(socket_fd);
            socket_fd = -1;
        }
    }
    return rcv_size;
}

int NetCommon::sendData(int socket_fd, const char* buf, const size_t size_to_tx) {
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
void NetCommon::GetPublicIp(char* buffer, std::size_t buf_size) const {
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

} // end of Network namespace