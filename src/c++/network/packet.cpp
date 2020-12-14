#include "packet.h"

namespace Network {

/********************************************** Constructors **********************************************/
Packet::Packet() 
    : msg_pkt{0}    // init pkt to all zeros
{
    // stub
}

Packet::~Packet() {
    // stub
}

/********************************************* Getters/Setters *********************************************/

const CommonPkt& Packet::getCurrentPkt() const {
    return msg_pkt;
}


/*************************************** Packet Read/Write Functions ***************************************/

json Packet::readPkt(const char* pkt_buf) const {
    return json::parse(pkt_buf);
}

CommonPkt Packet::interpretPkt(const json& recv_pkt) const {
    // see pkt_sample.json for format
    CommonPkt translated_pkt;
    translated_pkt.ACK              = recv_pkt["ACK"];
    translated_pkt.cntrl.led.red    = recv_pkt["control"]["led"]["red"];
    translated_pkt.cntrl.led.yellow = recv_pkt["control"]["led"]["yellow"];
    translated_pkt.cntrl.led.green  = recv_pkt["control"]["led"]["green"];
    translated_pkt.cntrl.led.blue   = recv_pkt["control"]["led"]["blue"];
    return translated_pkt;
}

const char* Packet::writePkt(const CommonPkt& pkt_to_send) const {
    // see pkt_sample.json for format
    json json_pkt = {
        "Control", {
            "LEDs", {
                "red",      pkt_to_send.cntrl.led.red,
                "yellow",   pkt_to_send.cntrl.led.yellow,
                "green",    pkt_to_send.cntrl.led.green,
                "blue",     pkt_to_send.cntrl.led.blue
            }
        },
        "ACK", pkt_to_send.ACK
    };
    return json_pkt.dump().c_str();
}


/********************************************* Helper Functions ********************************************/


} // end of Network namespace
