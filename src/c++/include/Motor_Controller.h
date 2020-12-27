#ifndef MOTOR_CONTROLLER_H
#define MOTOR_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>

// Our Includes
#include "constants.h"
#include "GPIO_Base.h"

// 3rd Party Includes
#include <wiringPi.h>
#include <softPwm.h>
#include <wiringPiI2C.h>

namespace RPI {
namespace gpio {
namespace Motor {

class MotorController : public GPIOBase {

    public:
        /********************************************** Constructors **********************************************/
        MotorController();
        virtual ~MotorController();

        /**
         * @brief Helps intialize the leds
         * @return ReturnCodes
         */
        virtual ReturnCodes init() const;

        /********************************************* Getters/Setters *********************************************/

        /********************************************* Motor Functions *********************************************/

    private:
        /******************************************** Private Variables ********************************************/

        const int motor_i2c_addr;               // the address for the robot's i2c motor module

        /********************************************* Helper Functions ********************************************/

}; // MotorController



}; // end of Motor namespace
}; // end of gpio namespace
}; // end of RPI namespace


#endif