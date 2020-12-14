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
    return json::parse(pkt_buf);
}

CommonPkt Packet::interpretPkt(const json& recv_pkt) const {
    // HACK: Somehow during transit between functions,
    // json object is wrapped by array (where json is only element)
    // I spent way too much time on this so just do everything from [0]
    CommonPkt translated_pkt;
    const json& data = recv_pkt[0];

    // see pkt_sample.json for format
    const json& control             = data["control"];
    const json& led                 = control["led"];
    translated_pkt.cntrl.led.red    = led["red"];
    translated_pkt.cntrl.led.yellow = led["yellow"];
    translated_pkt.cntrl.led.green  = led["green"];
    translated_pkt.cntrl.led.blue   = led["blue"];
    translated_pkt.ACK              = data["ACK"];
    return translated_pkt;
}

std::string Packet::writePkt(const CommonPkt& pkt_to_send) const {
    // see pkt_sample.json for format
    // https://github.com/nlohmann/json#json-as-first-class-data-type
    // have to double wrap {{}} to get it to work (each key-val needs to be wrapped)
    // key-values are seperated by commas not ':'
    json json_pkt = {{
        "control", {{
            "led", {
                {"red",      pkt_to_send.cntrl.led.red},
                {"yellow",   pkt_to_send.cntrl.led.yellow},
                {"green",    pkt_to_send.cntrl.led.green},
                {"blue",     pkt_to_send.cntrl.led.blue}
            }}
        }},
        {"ACK", pkt_to_send.ACK}
    };
    return json_pkt.dump();

    // TODO: use cbor or bson for faster/concise pkt transfers
    // std::vector<std::uint8_t> vec_char {json::to_cbor(json_pkt)};
    // return std::string(vec_char.begin(), vec_char.end());
}


/********************************************* Helper Functions ********************************************/


} // end of Network namespace
