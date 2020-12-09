// Our Includes
#include "CLI_Parser.h"

// local namespace modifications
using std::cout;
using std::cerr;
using std::endl;

namespace gpio {

/********************************************** Constructors **********************************************/
CLI_Parser::CLI_Parser(
    int _argc,
    char* _argv[],
    const std::vector<std::string>& mode_list,
    const std::vector<std::string>& color_list,
    const std::string name
)
    // instantiate parent constructor
    : CLI::App(name)
    , argc(_argc)
    , argv(_argv)
    , mode_list(mode_list)
    , color_list(color_list)
{
    // add flags/args to scan for
    addFlags();
}

/********************************************* Parse Functions *********************************************/
const CLI::Results::ParseResults& CLI_Parser::parse_flags() noexcept(false) {
    // actually parse flags
    try {
        CLI::App::parse(argc, argv);
    } catch  (const CLI::ParseError &e) {
        cerr << "=========== Failed to Parse CLI Flags! ===========" << endl;
        CLI::App::exit(e); // handles printing of error messages
        throw(e);
    }

    // return results (flags already know to store results into map)
    return cli_res;
}

/********************************************* Getters/Setters *********************************************/


/********************************************* Helper Functions ********************************************/
ReturnCodes CLI_Parser::addFlags() {
    add_option("-c,--colors", cli_res[CLI::Results::COLORS])
        ->description(
            "Which LEDs/Buttons to use (Comma-seperated): " + Helpers::createVecStr(color_list, ","))
        ->required(false)
        ->default_val("")
        // make sure each passed color is in the allowed list
        // return empty string == okay
        ->check([&](const std::string& color_str) {
            // first have to split the string to check each one
            const std::vector<std::string>& colors {Helpers::splitStr(',', color_str)};
            for (auto& color_to_check : colors) {
                bool isOkColor = false;
                for (auto& acceptable_color : color_list) {
                    if (color_to_check == acceptable_color) {
                        isOkColor = true;
                    }
                }

                // if this color was not okay, immediately return false
                if (!isOkColor) return std::string("Some colors are not in list");;
            }

            // if havent returned yet, all passed colors must be acceptable
            return std::string();
        })
        ;

    add_option("-m,--mode", cli_res[CLI::Results::MODE])
        ->description("Which action to perform: " + Helpers::createVecStr(mode_list, ","))
        ->required(true)
        ->check(CLI::IsMember(mode_list))
        ;

    add_option("-i,--interval", cli_res[CLI::Results::INTERVAL])
        ->description("The interval (in ms) between changing LEDs' states")
        ->required(false)
        ;

    add_option("-d,--duration", cli_res[CLI::Results::DURATION])
        ->description("How long the program should run (in ms)")
        ->required(false)
        ;

    add_option("-r,--rate", cli_res[CLI::Results::RATE])
        ->description("How fast the LEDs' intensity should change (1x, 2x, 3x...)")
        ->required(false)
        ;

    return ReturnCodes::Success;
}


}; // end of gpio namespace
