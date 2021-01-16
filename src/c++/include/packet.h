#ifndef PACKET_H
#define PACKET_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>
#include <netinet/in.h> // for ntons
#include <sstream> // for converting packets to strings
#include <atomic>
#include <condition_variable>

// Our Includes
#include "constants.h"

// 3rd Party Includes
#include <json.hpp>

namespace RPI {
namespace Network {

/************************************************ Common Packet Structs ******************************************/
// credit: https://stackoverflow.com/a/16523804/13933174

// for convenience inside Network namespace
using json = nlohmann::json;
using bson = std::vector<std::uint8_t>;

// defines max length a packet can have 
// typically uint16_t but camera frames are very large (will never have issues now)
using PktSize_t = std::uint64_t;

// packet structure follows format found @ctrl_pkt_sample.json
struct led_pkt_t {
    bool red;
    bool yellow;
    bool green;
    bool blue;

    led_pkt_t()
        : red{false}
        , yellow{false}
        , green{false}
        , blue{false}
        {}
}; // end of led_pkt_t

struct motor_pkt_t {
    bool forward;   // true if pressed forward key
    bool backward;  // true if pressed backward key
    bool right;     // true if pressed right key 
    bool left;      // true if pressed left key

    motor_pkt_t()
        : forward{false}
        , backward{false}
        , right{false}
        , left{false}
        {}
}; // end of motor_pkt_t

struct servo_pkt_t {
    int horiz;     // +/-/0 = right/left/unchanged
    int vert;      // +/-/0 = up/down/unchanged

    servo_pkt_t()
        : horiz{0}
        , vert{0}
        {}
}; // end of servo_pkt_t

struct camera_pkt_t {
    bool is_on;

    camera_pkt_t()
        : is_on{true} // have server start recording immediately on connect
        {}

}; // end of camera_pkt_t

struct CtrlData_t {
    led_pkt_t led;
    motor_pkt_t motor;
    servo_pkt_t servo;
    camera_pkt_t camera;
}; // end of control_t

struct CommonPkt {
    CtrlData_t cntrl;
    bool ACK;

    CommonPkt(
        const CtrlData_t& ctrl_data={},
        const bool ack={false}
    )
        : cntrl{ctrl_data}
        , ACK{ack}
        {}
}; // end of CommonPkt


/********************************************** ServerData Packet Material ****************************************/

// contains all data relevant to the camera
struct CamData_t {
    std::vector<unsigned char> img;
    int fps;
    int width;
    int height;

    // default to constant values
    CamData_t(
        const std::vector<unsigned char>& img={},
        int fps=Constants::Camera::VID_FRAMERATE,
        int width=Constants::Camera::FRAME_WIDTH,
        int height=Constants::Camera::FRAME_HEIGHT
    )
        : img{img}
        , fps{fps}
        , width{width}
        , height{height}
        {}
}; // end of CamData_t


/// contains all data produced by server to send to client
struct ServerData_t {
    // TODO: add battery, curr servo pos, ultrasonic, etc
    CamData_t cam;

    ServerData_t(const CamData_t& cam_data={})
        // default to defaults
        : cam{cam_data}
        {}
}; // end of ServerData_t


/************************************************ Header Packet Material ******************************************/

// struct to be sent prior to sending an actual data packet so its size & checksum can be known
// https://en.wikipedia.org/wiki/IPv4#Header
// mostly only checking/using total_length & checksum
struct HeaderPkt_t {
    std::uint8_t    ver_ihl;        // 4 bits version and 4 bits internet header length (ver=IPv<#>)
    std::uint8_t    tos;            // type of service
    PktSize_t       total_length;   // 
    std::uint16_t   id;             // 
    std::uint16_t   flags_fo;       // 3 bits flags and 13 bits fragment-offset
    std::uint8_t    ttl;            // time to live
    std::uint8_t    protocol;       // 
    std::uint16_t   checksum;       //
    std::uint32_t   src_addr;       // 
    std::uint32_t   dst_addr;       //

    // constructor makes conversion to HeaderPkt_t easy
    HeaderPkt_t     ();
    HeaderPkt_t     (std::istream& stream);
    // to string makes conversion from HeaderPkt_t easy
    std::string     toString();
    std::uint8_t    ihl() const;
    std::size_t     size() const;
};

// helper function to calculate the checksum of a data stream
std::uint16_t CalcChecksum(const void* data_buf, std::size_t size);


// defines what type of packet is being used
enum class PktType {
    Common,
    ServerData
}; // end of PktType

/**
 * @brief Type for a callback function that accepts a reference to the received pkt
 * @returns ReturnCodes::Success for no issues or ReturnCodes::Error if there was a problem
 */
using RecvCmnPktCallback = std::function<ReturnCodes(const CommonPkt&)>;

/**
 * @brief Type for a callback function that accepts a reference to the received server data pkt
 * @returns ReturnCodes::Success for no issues or ReturnCodes::Error if there was a problem
 */
using RecvServerPktCallback = std::function<ReturnCodes(const ServerData_t&)>;


/*************************************************** Packet Class **************************************************/

/**
 * @brief This class is responsible for writing and reading packets
 * 
 */
class Packet {
    public:
        /********************************************** Constructors **********************************************/
        Packet();
        virtual ~Packet();

        /********************************************* Getters/Setters *********************************************/

        // atomics
        bool getHasNewSendData() const;
        void setHasNewSendData(const bool new_state) const;

        virtual const CommonPkt& getCurrCmnPkt() const;
        virtual const ServerData_t& getCurrServerPkt() const;

        virtual ReturnCodes updatePkt(const CommonPkt& updated_pkt);
        virtual ReturnCodes updatePkt(const ServerData_t& updated_pkt);

        /**
         * @brief Get reference to the most recent camera data
         * @return Reference to the camera data struct
         * @note Needs to use a mutex bc of read/write race condition with server
         */
        virtual const CamData_t& getLatestCamData() const;

        /**
         * @brief Set the most recent camera data in memory
         * @param new_cam_data Reference to the new camera data
         * @return Success if no issues
         * @note Needs to use a mutex bc of read/write race condition with server
         */
        virtual ReturnCodes setLatestCamData(const CamData_t& new_cam_data);

        /*************************************** Packet Read/Write Functions ***************************************/
        // see https://github.com/nlohmann/json#binary-formats-bson-cbor-messagepack-and-ubjson

        /**
         * @brief Final endpoint for readCmnPkt -- can just call this one directly if have correct material
         * @param pkt_json The jsonified packet to parse
         * @return The parsed json packet in struct form
         */
        CommonPkt readCmnPkt(const json& pkt_json) const;

        /**
         * @brief Interprets a received packet and translates it to an easier type to deal with
         * @param pkt_buf A char array containing a stringified json/bson
         * @param size The size of the packet buffer
         * @param is_bson false is just a stringified/charified json. True if is a bson
         * @return The packet translated into the struct
         */
        CommonPkt readCmnPkt(const char* pkt_buf, const std::size_t size, const bool is_bson) const;

        /**
         * @brief Inbetween function that will just convert a buffered bson packet into a regular json
         * @param pkt_buf The bson packet buffer
         * @param size The size of the buffer
         * @return The jsonified packet buffer
         */
        json readCmnPkt(const char* pkt_buf, const std::size_t size) const;

        /**
         * @brief Final endpoint for readServerPkt -- can just call this one directly if have correct material
         * @param pkt_json The jsonified packet to parse
         * @return The parsed json packet in struct form
         */
        ServerData_t readServerPkt(const json& pkt_json) const;

        /**
         * @brief Interprets a received packet and translates it to an easier type to deal with
         * @param pkt_buf A char array containing a stringified json/bson
         * @param size The size of the packet buffer
         * @param is_bson false is just a stringified/charified json. True if is a bson
         * @return The packet translated into the struct
         */
        ServerData_t readServerPkt(const char* pkt_buf, const std::size_t size, const bool is_bson) const;

        /**
         * @brief Inbetween function that will just convert a buffered bson packet into a regular json
         * @param pkt_buf The bson packet buffer
         * @param size The size of the buffer
         * @return The jsonified packet buffer
         */
        json readServerPkt(const char* pkt_buf, const std::size_t size) const;


        /**
         * @brief Converts the packet from struct form to json form
         * (needed for sending packets without having to manually serialize them)
         * @param pkt The packet in struct form to convert
         * @return json The jsonified packet 
         */
        json convertPktToJson(const CommonPkt& pkt) const;
        json convertPktToJson(const ServerData_t& pkt) const;

        /**
         * @brief Construct & serialize a json (and then bson) packet to easily send over network
         * @param pkt_to_send The packet to send
         * @return The serialized bson string to send
         */
        std::string writePkt(const CommonPkt& pkt_to_send) const;
        std::string writePkt(const ServerData_t& pkt_to_send) const;
        std::string writePkt(const json& pkt_to_send) const;

    protected:
        // cv vars needed by derived classes in send loops
        std::condition_variable         cam_data_cv;        // notify in order for server to send camera data to client

    private:
        /******************************************** Private Variables ********************************************/

        mutable std::atomic_bool        has_new_send_data;  // true if there is new data to send

        // regular data packet variables
        CommonPkt                       latest_ctrl_pkt;    // holds the most up to date information from client
        mutable std::mutex              reg_pkt_mutex;      // controls access to the `latest_ctrl_pkt` data

        // camera pkt variables
        ServerData_t                    latest_server_pkt;  // holds the most up to date information from server
        mutable std::mutex              server_pkt_mutex;   // controls access to the `latest_server_pkt` data


        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Helper function that returns the values if the key exists 
         * (if dne, sets field to current packet's values)
         * @param type The type of packet being parsed
         * @param json_to_check The json to check if the key exists 
         * @param keys List of keys needed to access element (in order from root to branch of json)
         * @return The found element
         * @tparam rtnType The desired type for the return to be
         */
        template<typename rtnType>
        rtnType findIfExists(
            const PktType type,
            const json& json_to_check,
            const std::vector<std::string>& keys
        ) const;

}; // end of packet class


} // end of Network namespace

}; // end of RPI namespace

#endif
