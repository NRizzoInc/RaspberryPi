#ifndef LED_CONTROLLER_H
#define LED_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <chrono>
#include <atomic>
#include <thread>

// Our Includes
#include "map_helpers.hpp"
#include "string_helpers.hpp"
#include "timing.hpp"
#include "constants.h"

// 3rd Party Includes
#include <wiringPi.h>
#include <softPwm.h>

namespace gpio {
namespace LED {

/**
 * @brief Handles all LED operations
 */
class LEDController {
    public:
        /********************************************** Constructors **********************************************/
        LEDController();
        virtual ~LEDController();

        /**
         * @brief Helps intialize the leds
         * @return ReturnCodes
         */
        virtual ReturnCodes init() const;

        /********************************************* Getters/Setters *********************************************/
        /**
         * @brief: Gets a list of LED colors
         * @returns: Vector<string> of each color
         */
        std::vector<std::string> getLedColorList() const;

        /**
         * @brief Set whether the thread should stop
         * @param new_status The new status (true = stop, false = keep going) 
         * @return ReturnCodes
         * @note can be const because underlying bool is mutable
         */
        virtual ReturnCodes setShouldThreadExit(const bool new_status) const;
        /**
         * @brief Get whether the thread should stop
         * @return std::atomic_bool 
         */
        virtual const std::atomic_bool& getShouldThreadExit() const;

        virtual bool getIsInit() const;
        ReturnCodes setIsInit(const bool new_state) const;

        /********************************************* LED Functions *********************************************/
        /**
         * @brief Blink specific leds at the set interval (default to 1s) 
         * @param colors The list of colors to blink
         * @param interval The interval to blink the leds at (in ms)
         * @param duration How long to run for in ms (-1 = infinite)
         * @note Have to pass everything by reference do to function mapping requirements
         */
        void blinkLEDs(
            const std::vector<std::string>& colors,
            const unsigned int& interval=1000,
            const int& duration=-1,
            // not needed, but need to follow call guidlines for fn-mapping to work
            __attribute__((unused)) const unsigned int& rate=1
        ) const;

        /**
         * @brief Increase specific leds brightness at the set interval (default to 1s) 
         * @param colors The list of colors to blink
         * @param interval The interval to blink the leds at (in ms)
         * @param duration How long to run for in ms (-1 = infinite)
         * @param rate The rate at which the LEDs' intensity should change (i.e. 1x, 2x, 3x)
         * @note Have to pass everything by reference do to function mapping requirements
         */
        void LEDIntensity(
            const std::vector<std::string>& colors,
            const unsigned int& interval=1000,
            const int& duration=-1,
            const unsigned int& rate=1
        ) const;

        /**
         * @brief Turns a specific LED On/Off
         * @param pin_num The pin # of the LED to change
         * @param new_state true = ON, false = OFF
         * @return ReturnCodes 
         * @note Sets to binary of HIGH/LOW and does not deal with inbetween brightnesses
         */
        ReturnCodes setLED(const int pin_num, const bool new_state) const;
        /**
         * @brief Turns a specific LED On/Off
         * @param led_color The the color of the LED to change
         * @param new_state true = ON, false = OFF
         * @return ReturnCodes 
         * @note Sets to binary of HIGH/LOW and does not deal with inbetween brightnesses
         */
        ReturnCodes setLED(const std::string& led_color, const bool new_state) const;

    private:
        /******************************************** Private Variables ********************************************/
        // Map each color to a led's corresponding pin number
        const std::unordered_map<std::string, int> color_to_leds;
        /**
         * @brief Controls whether or not to stop blocking functions (i.e. blink)
         * @note Is mutable so that it can be modified in const functions safely
         */
        mutable std::atomic_bool stop_thread;
        mutable bool isInit;


        /********************************************* Helper Functions ********************************************/


}; // end of LEDController class


}; // end of LED namespace

}; // end of gpio namespace

#endif