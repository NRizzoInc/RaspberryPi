#include "packet.h"

namespace Network {

/********************************************** Constructors **********************************************/
Packet::Packet() {
    // stub
}

Packet::~Packet() {
    // stub
}

/********************************************* Getters/Setters *********************************************/


/*************************************** Packet Read/Write Functions ***************************************/

json Packet::readPkt(const char* pkt_buf) const {
    return json::from_bson(pkt_buf);
}

const char* Packet::writePkt(const json pkt_to_send) const {
    // std::vector<std::uint8_t> == const char*
    const bson pkt {json::to_bson(pkt_to_send)};
    return reinterpret_cast<const char*>(pkt.data());
}


/********************************************* Helper Functions ********************************************/


} // end of Network namespace
