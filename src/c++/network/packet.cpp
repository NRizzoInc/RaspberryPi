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
std::uint16_t CalcChecksum(const void* data_buf, std::size_t size) {
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
    : has_new_send_data{true}                // will be set false immediately after sending first message
{
    // stub
}

Packet::~Packet() {
    // stub
}

/********************************************* Getters/Setters *********************************************/

// atomics

bool Packet::getHasNewSendData() const {
    return has_new_send_data.load();
}

void Packet::setHasNewSendData(const bool new_state) const {
    has_new_send_data.store(new_state);
}



const CommonPkt& Packet::getCurrCmnPkt() const {
    // lock to make sure data can be gotten without new data being written
    std::unique_lock<std::mutex> lk{reg_pkt_mutex};
    return latest_ctrl_pkt;
}

const ServerData_t& Packet::getCurrServerPkt() const {
        // lock to make sure data can be gotten without new data being written
    std::unique_lock<std::mutex> lk{server_pkt_mutex};
    return latest_server_pkt;
}


ReturnCodes Packet::updatePkt(const CommonPkt& updated_pkt) {
    // lock to make sure data can be written without it trying to be read simultaneously
    std::unique_lock<std::mutex> lk{reg_pkt_mutex};
    latest_ctrl_pkt = updated_pkt;
    return ReturnCodes::Success;
}

ReturnCodes Packet::updatePkt(const ServerData_t& updated_pkt) {
    // lock to make sure data can be written without it trying to be read simultaneously
    std::unique_lock<std::mutex> lk{server_pkt_mutex};
    latest_server_pkt = updated_pkt;
    return ReturnCodes::Success;
}

const CamData_t& Packet::getLatestCamData() const {
    std::unique_lock<std::mutex> lk{server_pkt_mutex};
    return latest_server_pkt.cam;
}

ReturnCodes Packet::setLatestCamData(const CamData_t& new_cam_data) {
    std::unique_lock<std::mutex> lk{server_pkt_mutex};
    latest_server_pkt.cam = new_cam_data;
    // notify and set bool so send loop knows to send
    cam_data_cv.notify_all();
    setHasNewSendData(true);
    return ReturnCodes::Success;
}

/*************************************** Packet Read/Write Functions ***************************************/
// note: when using copy constructor cannot use {} because stores original json into an array

json Packet::readCmnPkt(const char* pkt_buf, const std::size_t size) const {
    // if empty, will just be empty json
    return json::from_bson(std::string(pkt_buf, size));
}


CommonPkt Packet::readCmnPkt(const char* pkt_buf, const std::size_t size, const bool is_bson) const {
    // if no data sent, just use current packet
    if (std::strlen(pkt_buf) == 0 || size == 0) {
        return getCurrCmnPkt();
    }

    // if valid str, parse and convert stringified bson to json covnert to standard pkt
    // (use bson for faster/concise pkt transfers)
    const json& pkt_json = is_bson ? readCmnPkt(pkt_buf, size) : json::parse(pkt_buf);
    return readCmnPkt(pkt_json);
}


CommonPkt Packet::readCmnPkt(const json& pkt_json) const {
    // see ctrl_pkt_sample.json for format
    // actually parse packet and save into struct
    CommonPkt pkt;

    const PktType type {PktType::Common};
    pkt.cntrl.led.red        = findIfExists<bool>(type, pkt_json, {"control",  "led",      "red"       });
    pkt.cntrl.led.yellow     = findIfExists<bool>(type, pkt_json, {"control",  "led",      "yellow"    });
    pkt.cntrl.led.green      = findIfExists<bool>(type, pkt_json, {"control",  "led",      "green"     });
    pkt.cntrl.led.blue       = findIfExists<bool>(type, pkt_json, {"control",  "led",      "blue"      });
    pkt.cntrl.motor.forward  = findIfExists<bool>(type, pkt_json, {"control",  "motor",    "forward"   });
    pkt.cntrl.motor.backward = findIfExists<bool>(type, pkt_json, {"control",  "motor",    "backward"  });
    pkt.cntrl.motor.right    = findIfExists<bool>(type, pkt_json, {"control",  "motor",    "right"     });
    pkt.cntrl.motor.left     = findIfExists<bool>(type, pkt_json, {"control",  "motor",    "left"      });
    pkt.cntrl.servo.horiz    = findIfExists<int> (type, pkt_json, {"control",  "servo",    "horiz"     });
    pkt.cntrl.servo.vert     = findIfExists<int> (type, pkt_json, {"control",  "servo",    "vert"      });
    pkt.cntrl.camera.is_on   = findIfExists<bool>(type, pkt_json, {"control",  "camera",   "is_on"     });
    pkt.ACK                  = findIfExists<bool>(type, pkt_json, {"ACK"});

    return pkt;
}

json Packet::readServerPkt(const char* pkt_buf, const std::size_t size) const {
    // if empty, will just be empty json
    return json::from_bson(std::string(pkt_buf, size));
}

ServerData_t Packet::readServerPkt(const char* pkt_buf, const std::size_t size, const bool is_bson) const {
    // if no data sent, just use current packet
    if (std::strlen(pkt_buf) == 0 || size == 0) {
        return getCurrServerPkt();
    }

    // if valid str, parse and convert stringified bson to json covnert to standard pkt
    // (use bson for faster/concise pkt transfers)
    const json& pkt_json = is_bson ? readServerPkt(pkt_buf, size) : json::parse(pkt_buf);
    return readServerPkt(pkt_json);
}

ServerData_t Packet::readServerPkt(const json& pkt_json) const {
    // see server_pkt_sample.json for format
    // actually parse packet and save into struct
    ServerData_t pkt;

    const PktType type {PktType::ServerData};

    // had to transfer image data as a string, so convert back to vector
    const std::string&  img_buf_str(findIfExists<std::string>(type, pkt_json, {"cam", "img" }));
    pkt.cam.img         = std::vector<unsigned char>(img_buf_str.begin(), img_buf_str.end());
    pkt.cam.fps         = findIfExists<int>(type, pkt_json, {"cam", "fps" });
    pkt.cam.width       = findIfExists<int>(type, pkt_json, {"cam", "width" });
    pkt.cam.height      = findIfExists<int>(type, pkt_json, {"cam", "height" });

    return pkt;
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
}

json Packet::convertPktToJson(const ServerData_t& pkt) const {
    // https://github.com/nlohmann/json#json-as-first-class-data-type
    // have to double wrap {{}} to get it to work (each key-val needs to be wrapped)
    // key-values are seperated by commas not ':'

    json json_pkt = {{
        "cam", {
            // cannot straight up send frame as a vector (encapsulate as string)
            {"img",         std::string{pkt.cam.img.begin(), pkt.cam.img.end()}},
            {"fps",         pkt.cam.fps},
            {"width",       pkt.cam.width},
            {"height",      pkt.cam.height},
        }}
    };
    return json_pkt;
}


std::string Packet::writePkt(const json& pkt_to_send) const {
    // see pkt_sample.json for format
    // use bson for faster/concise pkt transfers
    const bson& pkt_bson = json::to_bson(pkt_to_send);
    const std::string& pkt_bson_str = std::string{pkt_bson.begin(), pkt_bson.end()};
    return pkt_bson_str;
}


std::string Packet::writePkt(const CommonPkt& pkt_to_send) const {
    const json& pkt_json = convertPktToJson(pkt_to_send);
    return writePkt(pkt_json);
}

std::string Packet::writePkt(const ServerData_t& pkt_to_send) const {
    const json& pkt_json = convertPktToJson(pkt_to_send);
    return writePkt(pkt_json);
}



/********************************************* Helper Functions ********************************************/

template<typename rtnType>
rtnType Packet::findIfExists(
    const PktType type,
    const json& json_to_check,
    const std::vector<std::string>& keys
) const {
    // use current packet to fill in missing values
    // make copies of jsons to recur through
    // as parse thru keys, move deeper into the jsons so researching not needed

    // note: when using copy constructor cannot use {} because stores original json into an array
    // see: https://github.com/nlohmann/json/issues/1359

    // the received packet to parse through
    json recv_curr = json_to_check;

    // valid json of current stored pkt (source of truth)
    json curr_pkt = type == PktType::Common ? 
        convertPktToJson(getCurrCmnPkt()) : convertPktToJson(getCurrServerPkt());

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
