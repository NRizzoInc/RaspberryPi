#ifndef PACKET_H
#define PACKET_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

// Our Includes
#include "constants.h"

// 3rd Party Includes
#include <json.hpp>

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

struct control_t {
    led_pkt_t led;
}; // end of control_t

struct CommonPkt {
    control_t cntrl;
    bool ACK;
}; // end of CommonPkt

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

        /*************************************** Packet Read/Write Functions ***************************************/
        // see https://github.com/nlohmann/json#binary-formats-bson-cbor-messagepack-and-ubjson

        /**
         * @brief Interprets a received char buffer packet (stringified json)
         * @param pkt_buf A char array containing a stringified json
         * @return The packet in json form
         */
        json readPkt(const char* pkt_buf) const;

        /**
         * @brief Interprets a received packet and translates it to an easier type to deal with
         * Best paired with readPkt()
         * @param recv_json_pkt A json containing the received packet
         * @return The packet translated into the struct
         */
        CommonPkt interpretPkt(const json& recv_pkt) const;


        /**
         * @brief Construct & serialize a serialized json packet to easily send over network
         * @param pkt_to_send The packet to send
         * @return The serialized char array to send
         */
        const char* writePkt(const CommonPkt& pkt_to_send) const;

    private:
        /******************************************** Private Variables ********************************************/
        CommonPkt msg_pkt;  // holds the most up to date information from client (inits to all zeros)


        /********************************************* Helper Functions ********************************************/

}; // end of packet class


} // end of Network namespace

#endif
