// Standard Includes
#include <iostream>
#include <functional>
#include <csignal>

// Our Includes
#include "GPIO_Controller.h"
#include "CLI_Parser.h"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[]) {

    gpio::CLI_Parser cli_parser(argc, argv, "GPIO App");
    const CLI::ParseResults parse_res {cli_parser.parse_flags()};

    // create single static gpio obj to controll rpi
    // static needed so it can be accessed in lambda
    static gpio::GPIO_Controller gpio_obj;

    // setup ctrl+c handler w/ callback to stop threads
    std::signal(SIGINT, [](int signum) {
        cout << "Caught ctrl+c: " << signum << endl;
        gpio_obj.setShouldThreadExit(true);
    });

    cout << "Changing led intensity" << endl;
    gpio_obj.blinkLEDs({"red", "green"});
    // gpio_obj.LEDIntensity({"red", "green"});

    return 0;
}