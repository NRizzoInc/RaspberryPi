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
    add_option("-m,--mode", cli_res[CLI::Results::MODE])
        ->description("Which action to perform")
        ->required(true)
        ->check(CLI::IsMember(mode_list))
        ;

    const char delim {','};
    add_option("-c,--colors", cli_res[CLI::Results::COLORS])
        ->description("Which LEDs/Buttons to use (comma-seperated)")
        ->required(false)
        ->default_val("")
        // method for taking in multiple args => str-represented vector
        ->expected(0, color_list.size()-1)
        ->allow_extra_args() // allow mutliple inputs despite type=str
        ->delimiter(delim)
        ->check(CLI::IsMember(color_list))
        ->join(delim)
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
