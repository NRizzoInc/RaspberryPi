#ifndef CLI_PARSER_H
#define CLI_PARSER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>

// 3rd Party Includes
#include "CLI11.hpp"

// Our Includes
#include "constants.h"
#include "string_helpers.hpp"
#include "print_helpers.hpp"

namespace CLI::Results {
    // shortening of parse results mapping
    using ParseResults = std::unordered_map<std::string, std::string>;

    const std::string MODE      { "mode"     };
    const std::string COLORS    { "names"    };
    const std::string INTERVAL  { "interval" };
    const std::string RATE      { "rate"     };
    const std::string DURATION  { "duration" };
}; // end of CLI::Results namespace

namespace gpio {

/**
 * @brief Class that manages the GPIO's CLI
 */
class CLI_Parser : public CLI::App {
    public:
        /********************************************** Constructors **********************************************/
        
        /**
         * @brief Create the gpio parser with all required info
         * @param _argc The number of arguments
         * @param _argv An array of string-literals containing each argument\
         * @param mode_list A list of acceptable modes
         * @param color_list A list of available LEDs & Buttons to use
         * @param name The name of the parser (has a default)
         */
        explicit CLI_Parser(
            int _argc,
            char* _argv[],
            const std::vector<std::string>& mode_list,
            const std::vector<std::string>& color_list,
            const std::string name="GPIO CLI Parser"
        );

        /********************************************* Parse Functions *********************************************/
        /**
         * @brief Parses CLI and returns results in a map
         * @return The results in map
         * @note Will throw if encounters error while parsing cli
         */
        const CLI::Results::ParseResults& parse_flags() noexcept(false);


        /********************************************* Getters/Setters *********************************************/

    private:
        /******************************************** Private Variables ********************************************/
        // storage of number of cli argument
        int argc;
        // storage of cli argument
        char** argv;

        // the list of acceptable modes
        const std::vector<std::string>& mode_list;
        // the list of acceptable colors
        const std::vector<std::string>& color_list;

        CLI::Results::ParseResults cli_res;

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Adds all the required flags/arguments to the cli parser
         * @return ReturnCodes 
         */
        ReturnCodes addFlags();
};


}; // end of gpio namespace

#endif