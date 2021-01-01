#ifndef PACKET_H
#define PACKET_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <mutex>

// Our Includes
#include "constants.h"

// 3rd Party Includes
#include <json.hpp>

namespace RPI {
namespace Network {

// for convenience inside Network namespace
using json = nlohmann::json;
using bson = std::vector<std::uint8_t>;

// packet structure follows format found @pkt_sample.json
struct led_pkt_t {
    bool red;
    bool yellow;
    bool green;
    bool blue;
}; // end of led_pkt_t

struct motor_pkt_t {
    bool forward;   // true if pressed forward key
    bool backward;  // true if pressed backward key
    bool right;     // true if pressed right key 
    bool left;      // true if pressed left key
}; // end of motor_pkt_t

struct control_t {
    led_pkt_t led;
    motor_pkt_t motor;
}; // end of control_t

struct CommonPkt {
    control_t cntrl;
    bool ACK;
}; // end of CommonPkt

/**
 * @brief Type for a callback function that accepts a reference to the received pkt
 * @returns ReturnCodes::Success for no issues or ReturnCodes::Error if there was a problem
 */
using RecvPktCallback = std::function<ReturnCodes(const CommonPkt&)>;

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
        virtual const std::vector<char>& getLatestCamFrame();

        /**
         * @brief Set the latest frame from the camera video stream
         * @return Success if no issues
         * @note Needs to use a mutex bc of read/write race condition with server
         */
        virtual ReturnCodes setLatestCamFrame(const std::vector<char>& new_frame);

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
        CommonPkt msg_pkt;  // holds the most up to date information from client (inits to all zeros)

        // camera pkt variables
        std::vector<char>           latest_frame;       // contains the most up to date camera frame
        std::mutex                  frame_mutex;        // controls access to the `latest_frame` data


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
