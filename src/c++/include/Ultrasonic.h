#ifndef RPI_ULTRASONIC_H
#define RPI_ULTRASONIC_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>
#include <optional>

// Our Includes
#include "map_helpers.hpp"
#include "string_helpers.hpp"
#include "timing.hpp"
#include "constants.h"
#include "GPIO_Base.h"

// 3rd Party Includes
#include <wiringPi.h>


namespace RPI {
namespace gpio {
namespace Ultrasonic {

// these pins are defined by physical layout of robot
enum PinType {
    ECHO        = 22,       // returns current distance (input)
    TRIGGER     = 27,       // used to get current distance (output)
};

// querries to ultrasonic sensor should follow this pattern
// hence, sent and recv signals should be in this order
enum DistPulseOrder {
    First=true,
    Second=false,
};

class DistSensor : public GPIOBase {
    public:
        /********************************************** Constructors ***********************************************/

        /**
         * @brief DistSensor object that manages the RPI's ultrasonic sensor to get the robot's distance from objs.
         * @param verbosity If true, will print more information that is strictly necessary
         */
        DistSensor(const bool verbosity=false);
        virtual ~DistSensor();

        /**
         * @brief Helps intialize the ultransonic sensor's pins
         * @return ReturnCodes
         */
        virtual ReturnCodes init() const override;

        /********************************************* Getters/Setters *********************************************/


        /****************************************** Ultrasonic Functions *******************************************/

        /**
         * @brief Get the distance from the ultrasonic sensor
         * @return The distance of the robot from the nearest surface (relative to camera/ultrasonic sensor mount)
         * (std::nullopt if error)
         */
        std::optional<float> GetDistance() const;

        /**
         * @brief Test the ultrasonic distance sensor in a loop while it gets the distance
         * @note Have to pass everything by reference do to function mapping requirements
         */
        void testDistSensor(
            // not needed, but need to follow call guidlines for fn-mapping to work
            __attribute__((unused)) const std::vector<std::string>& colors={},
            const unsigned int& interval=1000,
            const int& duration=-1,
            __attribute__((unused)) const unsigned int& rate=1
        ) const;


    private:
        /******************************************** Private Variables ********************************************/


        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Waits until sensor detects the edge of the signal 
         * (Blocking until edge is seen or timeout is reached)
         * @param timeout How much time should pass before timing out on each edge
         * @returns Success if no issues, Timeout if timed out
         */
        ReturnCodes WaitForEdge(const bool edge_val, const std::chrono::steady_clock::duration timeout) const;

        /**
         * @brief Responsible for sending signal to trigger sensor to perform distance check
         * (Call WaitForEcho() afterwards)
         * @note Expected pulse
         */
        void SendTriggerPulse() const;

        /**
         * @brief Waits for data to be received by distance sensor
         * (Blocking until data received or timeout is reached)
         * @param timeout How much time should pass before timing out on each rising/falling edge (default to 1 second)
         * @returns The length of time it took ultrasonic signal to travel two-ways (std::nullopt if timeout)
         */
        std::optional<std::chrono::steady_clock::duration> WaitForEcho(
            const std::chrono::steady_clock::duration timeout=std::chrono::seconds(1)
        ) const;

}; // end of DistSensor


}; // end of Ultrasonic namespace
}; // end of gpio namespace
}; // end of RPI namespace


#endif
