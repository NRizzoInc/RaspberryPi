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
    return json::parse(pkt_buf);
}

const char* Packet::writePkt(const json pkt_to_send) const {
    return pkt_to_send.dump().c_str();
}


/********************************************* Helper Functions ********************************************/


} // end of Network namespace
