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
    const std::string name
)
    // instantiate parent constructor
    : CLI::App(name)
    , argc(_argc)
    , argv(_argv)
{
    // empty for now
}

/********************************************* Parse Functions *********************************************/


/********************************************* Getters/Setters *********************************************/


/********************************************* Helper Functions ********************************************/

}; // end of gpio namespace
