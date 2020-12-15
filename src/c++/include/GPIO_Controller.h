#ifndef GPIO_CONTROLLER_H
#define GPIO_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <thread>

// Our Includes
#include "string_helpers.hpp"
#include "map_helpers.hpp"
#include "constants.h"
#include "LED_Controller.h"
#include "Button_Controller.h"

// 3rd Party Includes
#include <wiringPi.h>

namespace gpio {

using MapParentMaps = std::unordered_map<
    std::string,
    std::pair<
        const LED::LEDMapVal&,         // led pin#
        const Button::BtnMapVal&       // button
    >
>;

/**
 * @brief Handles all GPIO related operations
 */
class GPIO_Controller : public LED::LEDController, public Button::ButtonController {
    public:
        /********************************************** Constructors **********************************************/
        GPIO_Controller();
        virtual ~GPIO_Controller();

        /********************************************* Getters/Setters *********************************************/
        /**
         * @brief: Gets a list of colors that are shared between LEDs & Buttons
         * @returns: Vector<string> of each color
         */
        std::vector<std::string> getPairColorList() const;

        /**
         * @brief Get the list of possible modes that map to functions
         * @return The possible modes (case-sensitive)
         */
        std::vector<std::string> getModes() const;

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
        ReturnCodes run(const CLI::Results::ParseResults& flags) const;

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
        const MapParentMaps                             color_to_led_btn_pairs;
        const Helpers::Map::ClassFnMap<GPIO_Controller> mode_to_action;         // maps a mode name to a gpio function
        mutable std::thread                             run_thread;             // thread that contains run()

        /********************************************* Helper Functions ********************************************/
        /**
         * @brief: Helps construct color_to_led_btn_pairs based on what colors they share
         * @returns: The constructed map of shared colors
         */
        MapParentMaps generateLedBtnPairs() const;


        /**
         * @brief Create a Fn Map object
         * @return The function map object 
         */
        Helpers::Map::ClassFnMap<GPIO_Controller> createFnMap() const;

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

#endif
