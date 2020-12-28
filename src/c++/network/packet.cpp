#include "packet.h"

namespace RPI {
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

CommonPkt Packet::readPkt(const char* pkt_buf) const {
    // see pkt_sample.json for format
    // if no data sent, just use current packet
    if (std::strlen(pkt_buf) == 0) {
        return getCurrentPkt();
    }

    // if valid str, parse and convert stringified json to json
    const json& data = json::parse(pkt_buf);
    CommonPkt translated_pkt;

    translated_pkt.cntrl.led.red        = findIfExists<bool>(data, {"control", "led",   "red"       });
    translated_pkt.cntrl.led.yellow     = findIfExists<bool>(data, {"control", "led",   "yellow"    });
    translated_pkt.cntrl.led.green      = findIfExists<bool>(data, {"control", "led",   "green"     });
    translated_pkt.cntrl.led.blue       = findIfExists<bool>(data, {"control", "led",   "blue"      });
    translated_pkt.cntrl.motor.forward  = findIfExists<int> (data, {"control", "motor", "forward"   });
    translated_pkt.cntrl.motor.backward = findIfExists<int> (data, {"control", "motor", "backward"  });
    translated_pkt.cntrl.motor.right    = findIfExists<int> (data, {"control", "motor", "right"     });
    translated_pkt.cntrl.motor.left     = findIfExists<int> (data, {"control", "motor", "left"      });
    translated_pkt.ACK                  = findIfExists<bool>(data, {"ACK"});

    return translated_pkt;
}

/// wraps to char buffer version of function
CommonPkt Packet::readPkt(json pkt_json) const {
    return readPkt(pkt_json.dump().c_str());
}


json Packet::convertPktToJson(const CommonPkt& pkt) const {
    // https://github.com/nlohmann/json#json-as-first-class-data-type
    // have to double wrap {{}} to get it to work (each key-val needs to be wrapped)
    // key-values are seperated by commas not ':'
    json json_pkt = {{
        "control", {
            {
                "led", {
                    {"red",         pkt.cntrl.led.red},
                    {"yellow",      pkt.cntrl.led.yellow},
                    {"green",       pkt.cntrl.led.green},
                    {"blue",        pkt.cntrl.led.blue}
                },
                "motor", {
                    {"forward",     pkt.cntrl.motor.forward},
                    {"backward",    pkt.cntrl.motor.backward},
                    {"right",       pkt.cntrl.motor.right},
                    {"left",        pkt.cntrl.motor.left}
                }
            }
        }},
        {"ACK", pkt.ACK}
    };
    return json_pkt;

    // TODO: use cbor or bson for faster/concise pkt transfers
    // std::vector<std::uint8_t> vec_char {json::to_cbor(json_pkt)};
    // return std::string(vec_char.begin(), vec_char.end());
}


std::string Packet::writePkt(const CommonPkt& pkt_to_send) const {
    // see pkt_sample.json for format
    return convertPktToJson(pkt_to_send).dump();
}


/********************************************* Helper Functions ********************************************/

template<typename rtnType>
rtnType Packet::findIfExists(const json& json_to_check, const std::vector<std::string>& keys) const {
    // use current packet to fill in missing values
    // make copies of jsons to recur through
    // as parse thru keys, move deeper into the jsons so researching not needed

    // note: when using copy constructor cannot use {} because stores original json into an array
    // see: https://github.com/nlohmann/json/issues/1359
    json recv_curr = json_to_check;                        // the received packet to parse through
    json curr_pkt  = convertPktToJson(getCurrentPkt());    // valid json of current stored pkt (source of truth)

    // loop condition checkers
    const std::size_t len_keys {keys.size()};
    std::size_t idx_counter {0};

    for (auto& key : keys) {
        // go deeper into json (select key and ignore rest)
        curr_pkt = curr_pkt[key]; // no worries with this not existing
        recv_curr = recv_curr.contains(key) ? recv_curr[key] : curr_pkt; // if dne, just use curr pkt (which has already zeroed in on the key)
        ++idx_counter;

        // first check that this is not the element to return
        // at the end of the key list
        if(idx_counter == len_keys) {
            return recv_curr.get<rtnType>();
        }
    }
    // never going to reach this point
    return curr_pkt;
}

} // end of Network namespace

}; // end of RPI namespace
