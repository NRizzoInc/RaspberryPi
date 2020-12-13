// Standard Includes
#include <iostream>
#include <functional>
#include <vector>
#include <csignal>
#include <thread>

// Our Includes
#include "GPIO_Controller.h"
#include "CLI_Parser.h"
#include "server.h"
#include "constants.h"
#include "string_helpers.hpp"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[]) {
    /* ============================================ Create GPIO Obj =========================================== */
    // create single static gpio obj to controll rpi
    // static needed so it can be accessed in ctrl+c lambda
    static const gpio::GPIO_Controller gpio_handler;

    /* ============================================ Parse CLI Flags =========================================== */
    // object that parses the command line inputs
    gpio::CLI_Parser cli_parser(
        argc,
        argv,
        gpio_handler.getModes(),
        gpio_handler.getLedColorList(),
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

    /* ============================================ Create Server  =========================================== */
    // static needed so it can be accessed in ctrl+c lambda
    static RPI::TcpServer server    {std::stoi(parse_res[CLI::Results::PORT])};

    /* ========================================= Create Ctrl+C Handler ======================================== */
    // setup ctrl+c handler w/ callback to stop threads
    std::signal(SIGINT, [](int signum) {
        cout << "Caught ctrl+c: " << signum << endl;
        gpio_handler.setShouldThreadExit(true);
        server.setExitCode(true);
    });

    /* ========================================== Initialize & Start ========================================= */

    // keep track of all threads to wait for
    std::vector<std::thread> thread_list;

    // start up gpio handler now that we have parse results
    gpio_handler.init();

    // run the selected gpio functionality 
    thread_list.push_back(std::thread{
        [&]() {
            gpio_handler.run(parse_res);
        }
    });

    // startup the tcp server in a thread to communicate with client
    thread_list.push_back(std::thread{
        [&]() {
            // TODO: set to false to not print data to terminal
            server.runServer(true);
        }
    });

    // make sure all threads complete
    for (auto& proc : thread_list) {
        if (proc.joinable()) {
            proc.join();
        }
    }

    return EXIT_SUCCESS;
}