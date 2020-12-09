// Standard Includes
#include <iostream>
#include <functional>
#include <csignal>

// Our Includes
#include "GPIO_Controller.h"
#include "CLI_Parser.h"
#include "string_helpers.hpp"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[]) {
    /* ============================================ Create GPIO Obj =========================================== */
    // create single static gpio obj to controll rpi
    // static needed so it can be accessed in lambda
    static gpio::GPIO_Controller gpio_handler;

    /* ============================================ Parse CLI Flags =========================================== */
    // object that parses the command line inputs
    gpio::CLI_Parser cli_parser(
        argc,
        argv,
        gpio_handler.getLedColorList(),
        gpio_handler.getModes(),
        "GPIO App"
    );
    // will have to convert string values to required type
    // note: will have to manually convert color str into list by splitting commas
    CLI::Results::ParseResults parse_res;

    try {
        parse_res = cli_parser.parse_flags();
    } catch (std::runtime_error& e) {
        return EXIT_FAILURE;
    }

    /* ========================================= Create Ctrl+C Handler ======================================== */
    // setup ctrl+c handler w/ callback to stop threads
    std::signal(SIGINT, [](int signum) {
        cout << "Caught ctrl+c: " << signum << endl;
        gpio_handler.setShouldThreadExit(true);
    });

    /* ========================================== Initialize & Start ========================================= */
    cout << "Initializing GPIO Board" << endl;
    gpio_handler.init();
    cout << "Changing led intensity" << endl;
    gpio_handler.blinkLEDs(Helpers::splitStr(',', parse_res[CLI::Results::COLORS]));
    // gpio_handler.LEDIntensity({"red", "green"});

    return 0;
}