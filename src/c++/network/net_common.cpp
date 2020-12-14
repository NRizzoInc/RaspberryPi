#include "net_common.h"

namespace Network {

using std::cout;
using std::cerr;
using std::endl;


/********************************************** Constructors **********************************************/

NetCommon::NetCommon()
    : should_exit{false}
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