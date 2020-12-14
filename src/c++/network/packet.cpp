#include "packet.h"

namespace Network {

// for convenience
using std::cout;
using std::cerr;
using std::endl;

/********************************************** Constructors **********************************************/
Packet::Packet() 
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

ReturnCodes Packet::updatePkt(const CommonPkt& updated_pkt) {
    msg_pkt = updated_pkt;
    return ReturnCodes::Success;
}


/*************************************** Packet Read/Write Functions ***************************************/

json Packet::readPkt(const char* pkt_buf) const {
    cout << "Recv: " << pkt_buf << endl;
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

std::string Packet::writePkt(const CommonPkt& pkt_to_send) const {
    // see pkt_sample.json for format
    // https://github.com/nlohmann/json#json-as-first-class-data-type
    // have to double wrap {{}} to get it to work (each key-val needs to be wrapped)
    // key-values are seperated by commas not ':'
    json json_pkt = {{
        "Control", {{
            "LEDs", {
                {"red",      pkt_to_send.cntrl.led.red},
                {"yellow",   pkt_to_send.cntrl.led.yellow},
                {"green",    pkt_to_send.cntrl.led.green},
                {"blue",     pkt_to_send.cntrl.led.blue}
            }}
        }},
        {"ACK", pkt_to_send.ACK}
    };
    return json_pkt.dump();
}


/********************************************* Helper Functions ********************************************/


} // end of Network namespace
