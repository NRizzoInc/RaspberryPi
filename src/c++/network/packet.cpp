#include "packet.h"

namespace RPI {
namespace Network {

// for convenience
using std::cout;
using std::cerr;
using std::endl;

/************************************************ Common Packet Structs ******************************************/

std::uint8_t HeaderPkt_t::ihl() const {
    return (ver_ihl & 0x0F);
}

std::size_t HeaderPkt_t::size() const {
    return ihl() * sizeof(std::uint32_t);
}

std::string HeaderPkt_t::toString() {
    // convert from standard network format (pay attention to which ones are longs = uint32_t)
    // total_length    = htonl(total_length);
    // id              = htons(id);
    // flags_fo        = htons(flags_fo);
    // checksum        = htons(checksum);
    // src_addr        = htonl(src_addr);
    // dst_addr        = htonl(dst_addr);

    std::ostringstream pkt_stream;
    pkt_stream.write((char*)&ver_ihl,      sizeof(ver_ihl));
    pkt_stream.write((char*)&tos,          sizeof(tos));
    pkt_stream.write((char*)&total_length, sizeof(total_length));
    pkt_stream.write((char*)&id,           sizeof(id));
    pkt_stream.write((char*)&flags_fo,     sizeof(flags_fo));
    pkt_stream.write((char*)&ttl,          sizeof(ttl));
    pkt_stream.write((char*)&protocol,     sizeof(protocol));
    pkt_stream.write((char*)&checksum,     sizeof(checksum));
    pkt_stream.write((char*)&src_addr,     sizeof(src_addr));
    pkt_stream.write((char*)&dst_addr,     sizeof(dst_addr));
    return pkt_stream.str();
}

HeaderPkt_t::HeaderPkt_t() {
    // stub to init everything to defaults (aka 0)
}

HeaderPkt_t::HeaderPkt_t(std::istream& stream) {
    stream.read((char*)&ver_ihl,      sizeof(ver_ihl));
    stream.read((char*)&tos,          sizeof(tos));
    stream.read((char*)&total_length, sizeof(total_length));
    stream.read((char*)&id,           sizeof(id));
    stream.read((char*)&flags_fo,     sizeof(flags_fo));
    stream.read((char*)&ttl,          sizeof(ttl));
    stream.read((char*)&protocol,     sizeof(protocol));
    stream.read((char*)&checksum,     sizeof(checksum));
    stream.read((char*)&src_addr,     sizeof(src_addr));
    stream.read((char*)&dst_addr,     sizeof(dst_addr));

    // convert to standard network format (pay attention to which ones are longs = uint32_t)
    // total_length    = ntohl(total_length);
    // id              = ntohs(id);
    // flags_fo        = ntohs(flags_fo);
    // checksum        = ntohs(checksum);
    // src_addr        = ntohl(src_addr);
    // dst_addr        = ntohl(dst_addr);
}

// credit: https://stackoverflow.com/a/23726131/13933174
std::uint16_t HeaderPkt_t::CalcChecksum(const void* data_buf, std::size_t size) {
    unsigned char x;
    unsigned short crc = 0xFFFF;

    while (size--){
        const unsigned char* ucharptr { static_cast<const unsigned char*>(data_buf) };
        // have to remove constness to increment position
        unsigned char* data_ptr { const_cast<unsigned char*>(ucharptr) };
        x = crc >> 8 ^ *(data_ptr++);
        x ^= x >> 4;
        crc = ( crc << 8 )
            ^ ( static_cast<unsigned short>(x << 12) )
            ^ ( static_cast<unsigned short>(x << 5)  )
            ^ ( static_cast<unsigned short>(x)       )
            ;
    }
    return crc;
}


/********************************************** Constructors **********************************************/
Packet::Packet()
    : latest_frame(Constants::Camera::FRAME_SIZE, '0') // init to black frame (0s) to make sure size != 0
{
    // stub
}

Packet::~Packet() {
    // stub
}

/********************************************* Getters/Setters *********************************************/

const CommonPkt& Packet::getCurrentPkt() const {
    // lock to make sure data can be gotten without new data being written
    std::unique_lock<std::mutex> lk{reg_pkt_mutex};
    return latest_ctrl_pkt;
}

ReturnCodes Packet::updatePkt(const CommonPkt& updated_pkt) {
    // lock to make sure data can be written without it trying to be read simultaneously
    std::unique_lock<std::mutex> lk{reg_pkt_mutex};
    latest_ctrl_pkt = updated_pkt;
    return ReturnCodes::Success;
}

const std::vector<unsigned char>& Packet::getLatestCamFrame() const {
    // lock to make sure data can be gotten without new data being written
    std::unique_lock<std::mutex> lk{frame_mutex};
    return latest_frame;
}


ReturnCodes Packet::setLatestCamFrame(const std::vector<unsigned char>& new_frame) {
    // lock to make sure data can be written without it trying to be read simultaneously
    std::unique_lock<std::mutex> lk{frame_mutex};
    latest_frame = new_frame;
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

    translated_pkt.cntrl.led.red        = findIfExists<bool>(data, {"control",  "led",      "red"       });
    translated_pkt.cntrl.led.yellow     = findIfExists<bool>(data, {"control",  "led",      "yellow"    });
    translated_pkt.cntrl.led.green      = findIfExists<bool>(data, {"control",  "led",      "green"     });
    translated_pkt.cntrl.led.blue       = findIfExists<bool>(data, {"control",  "led",      "blue"      });
    translated_pkt.cntrl.motor.forward  = findIfExists<bool>(data, {"control",  "motor",    "forward"   });
    translated_pkt.cntrl.motor.backward = findIfExists<bool>(data, {"control",  "motor",    "backward"  });
    translated_pkt.cntrl.motor.right    = findIfExists<bool>(data, {"control",  "motor",    "right"     });
    translated_pkt.cntrl.motor.left     = findIfExists<bool>(data, {"control",  "motor",    "left"      });
    translated_pkt.cntrl.servo.horiz    = findIfExists<int> (data, {"control",  "servo",    "horiz"     });
    translated_pkt.cntrl.servo.vert     = findIfExists<int> (data, {"control",  "servo",    "vert"      });
    translated_pkt.cntrl.camera.is_on   = findIfExists<bool>(data, {"control",  "camera",   "is_on"     });
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
            {"led", {
                {"red",         pkt.cntrl.led.red},
                {"yellow",      pkt.cntrl.led.yellow},
                {"green",       pkt.cntrl.led.green},
                {"blue",        pkt.cntrl.led.blue}
            }},
            {"motor", {
                {"forward",     pkt.cntrl.motor.forward},
                {"backward",    pkt.cntrl.motor.backward},
                {"right",       pkt.cntrl.motor.right},
                {"left",        pkt.cntrl.motor.left}
            }},
            {"servo", {
                {"horiz",       pkt.cntrl.servo.horiz},
                {"vert",        pkt.cntrl.servo.vert},
            }},
            {"camera", {
                {"is_on",       pkt.cntrl.camera.is_on}
            }}
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
