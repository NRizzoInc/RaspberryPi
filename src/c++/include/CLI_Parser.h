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