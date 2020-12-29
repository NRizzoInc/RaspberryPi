#ifndef GPIO_CONTROLLER_H
#define GPIO_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <thread>
// block destructor with mutex so that thread can be created prior to joining
#include <atomic>
#include <mutex>
#include <condition_variable>

// Our Includes
#include "string_helpers.hpp"
#include "map_helpers.hpp"
#include "constants.h"
#include "LED_Controller.h"
#include "Button_Controller.h"
#include "Motor_Controller.h"
#include "packet.h"

// 3rd Party Includes
#include <wiringPi.h>

namespace RPI {
namespace gpio {

using MapParentMaps = std::unordered_map<
    std::string,
    std::pair<
        const LED::LEDMapVal&,         // led pin#
        const Button::BtnMapVal&       // button
    >
>;

class GPIOController; // forward declare for mapping
using ModeMap = Helpers::Map::ClassFnMap<GPIOController>;

/**
 * @brief Handles all GPIO related operations
 */
class GPIOController : public LED::LEDController, public Button::ButtonController, public Motor::MotorController {
    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a gpio controller responsible for managing all things relating to the gpio
         * (i.e. buttons, leds, motors, etc...)
         * @param motor_i2c_addr The address of the motor controller i2c board
         */
        GPIOController(const std::uint8_t motor_i2c_addr);
        virtual ~GPIOController();

        /********************************************* Getters/Setters *********************************************/
        /**
         * @brief: Gets a list of colors that are shared between LEDs & Buttons
         * @returns: Vector<string> of each color
         */
        static std::vector<std::string> getPairColorList();

        /**
         * @brief Get the list of possible modes that map to functions
         * @return The possible modes (case-sensitive)
         */
        static std::vector<std::string> getModes();

        /**
         * @brief Determines if all GPIO modules are init
         * @return True if everything is good to go
         */
        bool getIsInit() const override;

        /*********************************************** GPIO Helpers **********************************************/

        /**
         * @brief Wrapper for base classes init functions
         * @return ReturnCodes Success if gpio board is init
         */
        ReturnCodes init() const override;

        /**
         * @brief Set whether the thread should stop
         * @param new_status The new status (true = stop, false = keep going) 
         * @return ReturnCodes
         * @note can be const because underlying bool is mutable
         */
        ReturnCodes setShouldThreadExit(const bool new_status) const override;

        /**
         * @brief Handles the execution of the selected function in a thread
         * @param flags Mapping contianing all command line flag values needed to call
         * correct function with correct params
         * @param args Additional args to unpack for the function call
         * @return ReturnCodes
         */
        ReturnCodes run(const CLI::Results::ParseResults& flags);

        /**
         * @brief Wrapper for wrapping and closing functions that need to be called
         * before end object goes out of scope.
         * @return ReturnCodes 
         */
        virtual ReturnCodes cleanup();

        ReturnCodes gpioHandlePkt(const Network::CommonPkt& pkt) const;

    private:
        /******************************************** Private Variables ********************************************/
        /**
         * @brief Maps a color to the values in the parent maps
         * {
         *      "color": {
         *          pin#, // leds
         *          {pin#, pressed_state}  // buttons
         *      }
         * }
         * 
         */
        static const ModeMap                            mode_to_action;         // maps a mode name to a gpio function
        std::thread                                     run_thread;             // thread that contains run()
        std::atomic_bool                                started_thread;         // wait for thread start before joining
        std::mutex                                      thread_mutex;
        std::condition_variable                         thread_cv;              // unblock if client has pkt to send
        bool                                            has_cleaned_up;         // ensures cleanup doesnt happen twice
        
        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Create a Fn Map object
         * @return The function map object
         * @note static so that it can be used to create the static member variable
         */
        static ModeMap createFnMap();

        /**
         * @brief Helper function that ... literally does nothing
         * @note Needed for map to have an option to test other features/do nothing
         */
        void doNothing() const;

        /**
         * @brief Wrapper for FnMap's searchAndCall() so that it can be bound for lambda
         * @note Without this, would ahve to copy "this" object by value to pass into lambda
         */
        void callSelFn(
            const std::string& mode,
            const std::vector<std::string>& colors,
            const unsigned int& interval,
            const int& duration,
            const unsigned int& rate
        ) const;

};


}; // end of gpio namespace

}; // end of RPI namespace

#endif
