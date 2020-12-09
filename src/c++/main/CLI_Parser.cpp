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
        cerr << "Failed to Parse CLI Flags: " << e.what() << endl;
        CLI::App::exit(e); // handles printing of error messages
        throw(e);
    }

    // return results (flags already know to store results into map)
    return cli_res;
}

/********************************************* Getters/Setters *********************************************/


/********************************************* Helper Functions ********************************************/
ReturnCodes CLI_Parser::addFlags() {

}


}; // end of gpio namespace
