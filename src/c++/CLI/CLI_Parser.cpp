// Our Includes
#include "CLI_Parser.h"

// local namespace modifications
using std::cout;
using std::cerr;
using std::endl;

namespace RPI {
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
    : ::CLI::App(name)
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
        ::CLI::App::parse(argc, argv);
    } catch  (const ::CLI::ParseError &e) {
        cerr << "=========== Failed to Parse CLI Flags! ===========" << endl;
        ::CLI::App::exit(e); // handles printing of error messages
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
        ->check(::CLI::IsMember(mode_list))
        // make sure only 1 mode is ever taken
        ->expected(1)
        ->take_first()
        ;

    const char delim {','};
    add_option("-c,--colors", cli_res[CLI::Results::COLORS])
        ->description("Which LEDs/Buttons to use (comma-seperated)")
        ->required(false)
        // method for taking in multiple args => str-represented vector
        ->expected(0, color_list.size())
        ->allow_extra_args() // allow mutliple inputs despite type=str
        ->delimiter(delim)
        ->check(::CLI::IsMember(color_list))
        ->join(delim)
        ;

    add_option("-i,--interval", cli_res[CLI::Results::INTERVAL])
        ->description("The interval (in ms) between changing LEDs' states")
        ->required(false)
        ->default_val("1000")
        ;

    add_option("-d,--duration", cli_res[CLI::Results::DURATION])
        ->description("How long the program should run (in ms)")
        ->required(false)
        ->default_val("-1")
        ;

    add_option("-r,--rate", cli_res[CLI::Results::RATE])
        ->description("How fast the LEDs' intensity should change (1x, 2x, 3x...)")
        ->required(false)
        ->default_val("1")
        ;

    /**************************************** Networking Flags ****************************************/

    // mark them both as needing mode_opt bc its results impact them
    add_option("-a,--ip", cli_res[CLI::Results::IP])
        ->description("The server's ip address")
        ->required(false)
        ->default_val("127.0.0.1")
        ->check(::CLI::ValidIPV4)
        ;

    add_option("-p,--net-port", cli_res[CLI::Results::NET_PORT])
        ->description("The server's/client's port number")
        ->required(false)
        ->default_val("55555")
        ->check(::CLI::Range(1024, 65535))
        ;

    /****************************************** Web App Flags *****************************************/

    add_option("--web-port", cli_res[CLI::Results::WEB_PORT])
        ->description("The web-app's port number")
        ->required(false)
        ->default_val("5001")
        ->check(::CLI::Range(1024, 65535))
        ;

    return ReturnCodes::Success;
}


}; // end of gpio namespace

}; // end of RPI namespace
