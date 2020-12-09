#ifndef CLI_PARSER_H
#define CLI_PARSER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>

// 3rd Party Includes
#include "CLI11.hpp"

// Our Includes
#include "constants.h"

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
         * @param _argv An array of string-literals containing each argument
         * @param name The name of the parser (has a default)
         */
        explicit CLI_Parser(
            int _argc,
            char* _argv[],
            const std::string name="GPIO CLI Parser"
        );

        /**
         * @Brief: Parses command line flags and returns results in json"
         * @Args:
         *      name: the name of the parser
         *      parser_description: Description of the parser
         *      argc: The number of arguments
         *      argv: The arguments
         * @Return: json containing collated flag results. May need to convert "values" using .get<T>() on elements
         * @Note: These arguments should be taken directly from int main(argc, argv)
         * @Note: using CLI11 for command-line parsing: https://cliutils.gitlab.io/CLI11Tutorial/
         */
        // nlohmann::json parse_flags(int argc, char* argv[], std::string parser_name) const;

        /********************************************* Parse Functions *********************************************/

        /********************************************* Getters/Setters *********************************************/

    private:
        /******************************************** Private Variables ********************************************/
        // storage of number of cli argument
        int argc;
        // storage of cli argument
        char** argv;

        /********************************************* Helper Functions ********************************************/
};


}; // end of gpio namespace

#endif