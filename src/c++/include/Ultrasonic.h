#ifndef RPI_ULTRASONIC_H
#define RPI_ULTRASONIC_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <chrono>
#include <thread>

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


    private:
        /******************************************** Private Variables ********************************************/


        /********************************************* Helper Functions ********************************************/

}; // end of DistSensor


}; // end of Ultrasonic namespace
}; // end of gpio namespace
}; // end of RPI namespace


#endif
