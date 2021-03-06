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
#include <condition_variable> // block with mutex until new data set
#include <atomic>

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

// packet structure follows format found @pkt_sample.json
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

struct control_t {
    led_pkt_t led;
    motor_pkt_t motor;
    servo_pkt_t servo;
    camera_pkt_t camera;
}; // end of control_t

struct CommonPkt {
    control_t cntrl;
    bool ACK;
}; // end of CommonPkt


/*********************************************** Server Data Packet Structs *****************************************/

struct ultrasonic_pkt_t {
    float dist;

    ultrasonic_pkt_t()
        : dist{0.0}
        {}
}; // end of ultrasonic_pkt_t

struct SrvDataPkt {
    ultrasonic_pkt_t ultrasonic;
    bool ACK;
}; // end of SrvDataPkt

/************************************************** Header Packet Structs *******************************************/


// struct to be sent prior to sending an actual data packet so its size & checksum can be known
// https://en.wikipedia.org/wiki/IPv4#Header
// mostly only checking/using total_length & checksum
struct HeaderPkt_t {
    std::uint8_t    ver_ihl;        // 4 bits version and 4 bits internet header length (ver=IPv<#>)
    std::uint8_t    tos;            // type of service
    std::uint32_t   total_length;   // typically uint16_t but camera frames are very large (>100,000)
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
    std::uint16_t   CalcChecksum(const void* data_buf, std::size_t size);
    std::uint8_t    ihl() const;
    std::size_t     size() const;
};

enum class PktType {
    SrvData,
    Common,
};


/**
 * @brief Type for a callback function that accepts a reference to the received pkt
 * @returns ReturnCodes::Success for no issues or ReturnCodes::Error if there was a problem
 */
using RecvPktCallback = std::function<ReturnCodes(const CommonPkt&)>;


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

        virtual const CommonPkt& getCurrentCmnPkt() const;
        virtual const SrvDataPkt& getCurrentSrvPkt() const;

        /**
         * @brief 
         * (notifies cv ``)
         * @param updated_pkt 
         * @return ReturnCodes 
         */
        virtual ReturnCodes updatePkt(const CommonPkt& updated_pkt);
        virtual ReturnCodes updatePkt(const SrvDataPkt& updated_pkt);

        /**
         * @brief Get the latest frame from the camera video stream
         * @return Reference to the char buffer in the form of a char vector
         * @note Needs to use a mutex bc of read/write race condition with server
         */
        virtual const std::vector<unsigned char>& getLatestCamFrame() const;

        /**
         * @brief Set the latest frame from the camera video stream
         * @return Success if no issues
         * @note Needs to use a mutex bc of read/write race condition with server
         */
        virtual ReturnCodes setLatestCamFrame(const std::vector<unsigned char>& new_frame);

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
         * @brief Final endpoint for readCmnPkt -- can just call this one directly if have correct material
         * @param pkt_json The jsonified packet to parse
         * @return The parsed json packet in struct form
         */
        SrvDataPkt readSrvPkt(const json& pkt_json) const;

        /**
         * @brief Interprets a received packet and translates it to an easier type to deal with
         * @param pkt_buf A char array containing a stringified json/bson
         * @param size The size of the packet buffer
         * @param is_bson false is just a stringified/charified json. True if is a bson
         * @return The packet translated into the struct
         */
        SrvDataPkt readSrvPkt(const char* pkt_buf, const std::size_t size, const bool is_bson) const;

        /**
         * @brief Inbetween function that will just convert a buffered bson packet into a regular json
         * @param pkt_buf The bson packet buffer
         * @param size The size of the buffer
         * @return The jsonified packet buffer
         */
        json readSrvPkt(const char* pkt_buf, const std::size_t size) const;

        /**
         * @brief Converts the packet from struct form to json form
         * (needed for sending packets without having to manually serialize them)
         * @param pkt The packet in struct form to convert
         * @return json The jsonified packet 
         */
        json convertPktToJson(const CommonPkt& pkt) const;
        json convertPktToJson(const SrvDataPkt& pkt) const;

        /**
         * @brief Construct & serialize a json (and then bson) packet to easily send over network
         * @param pkt_to_send The packet to send
         * @return The serialized bson string to send
         */
        std::string writePkt(const CommonPkt& pkt_to_send) const;
        std::string writePkt(const SrvDataPkt& pkt_to_send) const;
        std::string writePkt(const json& pkt_to_send) const;

    protected:
        // vars needed by both client/server for checking whether they are ready/able to send pkts
        std::atomic_bool            cmn_pkt_ready;      // ready to send new common packet
        std::atomic_bool            cam_pkt_ready;      // ready to send new camera data packet
        std::atomic_bool            srv_pkt_ready;      // ready to send new server data packet
        std::condition_variable     has_new_cmn_data;   // true if client/server needs to send new common msg
        std::condition_variable     has_new_cam_data;   // true if client/server needs to send new camera data
        std::condition_variable     has_new_srv_data;   // true if client/server needs to send new server data msg
        mutable std::mutex          cmn_data_pkt_mutex; // controls access to the `latest_ctrl_pkt` data
        mutable std::mutex          cam_data_pkt_mutex; // controls access to the `latest_frame` data
        mutable std::mutex          srv_data_pkt_mutex; // controls access to the `latest_srv_data_pkt` data

    private:
        /******************************************** Private Variables ********************************************/

        // regular data packet variables
        CommonPkt                       latest_ctrl_pkt;    // holds the most up to date information from client

        // camera pkt variables
        std::vector<unsigned char>      latest_frame;       // contains the most up to date camera frame
        mutable std::mutex              frame_mutex;        // controls access to the `latest_frame` data

        // server data packet variables
        SrvDataPkt                      latest_srv_data_pkt;// holds the most up to date information to send to client

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Helper function that returns the values if the key exists 
         * (if dne, sets field to current packet's values)
         * @param json_to_check The json to check if the key exists 
         * @param keys List of keys needed to access element (in order from root to branch of json)
         * @return The found element
         * @tparam rtnType The desired type for the return to be
         */
        template<typename rtnType>
        rtnType findIfExists(const json& json_to_check, const std::vector<std::string>& keys) const;

}; // end of packet class


} // end of Network namespace

}; // end of RPI namespace

#endif
