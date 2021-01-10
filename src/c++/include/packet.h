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


/************************************ Client -> Server Control Packet Structs ******************************/
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

// contains key information from client about what server should do (hence "control")
struct control_t {
    led_pkt_t led;
    motor_pkt_t motor;
    servo_pkt_t servo;
    camera_pkt_t camera;
}; // end of control_t


/************************************ Server -> Client Data Packet Structs ******************************/
// packet structure follows format found @server_pkt_sample.json

/// contains key information on the camera data coming from the server
struct cam_frame_t {
    PktSize_t framesize;            // the size of the video frame's buffer
    std::uint8_t fps;               // the video's frame rate
    std::uint16_t width;            // the width of the video's frame (max would be 1080p)
    std::uint16_t height;           // the height of the video's frame (max would be 1080p)
    unsigned char* frame;           // buffer containing the video frame

    cam_frame_t()
        : framesize{0}
        , fps{Constants::Camera::VID_FRAMERATE}
        , width{Constants::Camera::FRAME_WIDTH}
        , height{Constants::Camera::FRAME_HEIGHT}
        , frame{}
        {}
}; // end of cam_frame_t

// contains data from server that needs to be relayed to client
struct ServerDataPkt_t {
    cam_frame_t camera;
}; // ServerDataPkt_t


/************************************ Final Common Packet ******************************/
struct CommonPkt {
    control_t cntrl;                        // contains data from client that server wants
    ServerDataPkt_t server_data;            // contains data from server that client wants
    bool ACK;
}; // end of CommonPkt


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

        virtual const CommonPkt& getCurrentPkt() const;

        virtual ReturnCodes updatePkt(const CommonPkt& updated_pkt);

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
         * @brief Interprets a received packet and translates it to an easier type to deal with
         * @param pkt_buf A char array containing a stringified json
         * @return The packet translated into the struct
         */
        CommonPkt readPkt(const char* pkt_buf) const;
        /**
         * @brief Interprets a received packet and translates it to an easier type to deal with
         * @param pkt_json A json containing the packet info
         * @return The packet translated into the struct
         */
        CommonPkt readPkt(json pkt_json) const;


        json convertPktToJson(const CommonPkt& pkt) const;

        /**
         * @brief Construct & serialize a serialized json packet to easily send over network
         * @param pkt_to_send The packet to send
         * @return The serialized json string to send
         */
        std::string writePkt(const CommonPkt& pkt_to_send) const;

    private:
        /******************************************** Private Variables ********************************************/

        // regular data packet variables
        CommonPkt                       latest_ctrl_pkt;    // holds the most up to date information from client
        mutable std::mutex              reg_pkt_mutex;      // controls access to the `latest_ctrl_pkt` data

        // camera pkt variables
        std::vector<unsigned char>      latest_frame;       // contains the most up to date camera frame
        mutable std::mutex              frame_mutex;        // controls access to the `latest_frame` data


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
