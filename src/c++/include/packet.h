#ifndef PACKET_H
#define PACKET_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

// Our Includes

// 3rd Party Includes
#include <json.hpp>

namespace Network {

// for convenience inside Network namespace
using json = nlohmann::json;
using bson = std::vector<std::uint8_t>;

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

        /*************************************** Packet Read/Write Functions ***************************************/
        // see https://github.com/nlohmann/json#binary-formats-bson-cbor-messagepack-and-ubjson

        /**
         * @brief Interprets a received char buffer packet (stringified json)
         * @param pkt_buf A char array containing a stringified json
         * @return The packet in json form
         */
        json readPkt(const char* pkt_buf) const;

        /**
         * @brief Construct & serialize a json packet
         * @param pkt_to_send The json packet to send
         * @return The serialized char array to send
         */
        const char* writePkt(const json pkt_to_send) const;

    private:
        /******************************************** Private Variables ********************************************/


        /********************************************* Helper Functions ********************************************/

}; // end of packet class


} // end of Network namespace

#endif
