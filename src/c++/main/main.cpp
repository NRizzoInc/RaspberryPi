// Standard Includes
#include <iostream>
#include <csignal> // for ctrl+c signal handling

// 3rd Party Includes

// Our Includes
#include "GPIO_Controller.h"
#include "CLI_Parser.h"
#include "server.h"
#include "client.h"
#include "tcp_base.h"
#include "tcp_base.h"
#include "constants.h"
#include "string_helpers.hpp"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[]) {
    /* ============================================ Create GPIO Obj =========================================== */
    // create single static gpio obj to controll rpi
    // static needed so it can be accessed in ctrl+c lambda
    static RPI::gpio::GPIO_Controller gpio_handler;

    /* ============================================ Parse CLI Flags =========================================== */
    // object that parses the command line inputs
    RPI::gpio::CLI_Parser cli_parser(
        argc,
        argv,
        gpio_handler.getModes(),
        gpio_handler.getLedColorList(),
        "GPIO App"
    );
    // will have to convert string values to required type
    // note: will have to manually convert color str into list by splitting commas
    RPI::CLI::Results::ParseResults parse_res;

    try {
        parse_res = cli_parser.parse_flags();
    } catch (std::runtime_error& e) {
        return EXIT_FAILURE;
    }
    // determine if dealing with server or client
    const bool is_client {parse_res[RPI::CLI::Results::MODE] == "client"};
    const bool is_server {parse_res[RPI::CLI::Results::MODE] == "server"};
    const bool is_net    { is_client || is_server }; // true if client or server

    /* ======================================== Create Server OR Client ======================================= */
    // static needed so it can be accessed in ctrl+c lambda
    // if is_client, do not init the server (and vice-versa)
    const int port {std::stoi(parse_res[RPI::CLI::Results::PORT])}; // dont convert this twice
    static std::unique_ptr<RPI::Network::TcpBase> net_agent {
        is_client ?
            (RPI::Network::TcpBase*) new RPI::Network::TcpClient{parse_res[RPI::CLI::Results::IP], port, is_client} :
            (RPI::Network::TcpBase*) new RPI::Network::TcpServer{port, !is_client}
    };

    /* ========================================= Create Ctrl+C Handler ======================================== */
    // setup ctrl+c handler w/ callback to stop threads
    std::signal(SIGINT, [](int signum) {
        cout << "Caught ctrl+c: " << signum << endl;
        if(gpio_handler.setShouldThreadExit(true) != RPI::ReturnCodes::Success) {
            cerr << "Error: Failed to stop gpio thread" << endl;
        }
        
        if(net_agent->setExitCode(true) != RPI::ReturnCodes::Success) {
            cerr << "Error: Failed to stop network thread" << endl;
        }
    });

    /* ========================================== Initialize & Start ========================================= */

    // if not the client: init and run gpio functionality
    if (!is_client) {
        // start up gpio handler now that we have parse results
        gpio_handler.init();

        // run the selected gpio functionality (non-blocking thread handled by class)
        gpio_handler.run(parse_res);

        // set recv to handle when getting packets
        net_agent->setRecvCallback([&](const RPI::Network::CommonPkt& pkt)->RPI::ReturnCodes{
            return gpio_handler.gpioHandlePkt(pkt);
        });

    }

    // startup client or server in a thread
    if (is_net) {
        // TODO: set to false to not print data to terminal
        cout << "Started net agent" << endl;
        net_agent->runNetAgent(true);
    }

    /* =============================================== Cleanup =============================================== */
    if(net_agent->cleanup() != RPI::ReturnCodes::Success) {
        const std::string net_agent_name {is_client ? "client" : "server"};
        cerr << "Failed to cleanup " << net_agent_name << " " << endl;
        return EXIT_FAILURE;
    }
    if(gpio_handler.cleanup() != RPI::ReturnCodes::Success) {
        cerr << "Failed to cleanup gpio" << endl;
        return EXIT_FAILURE;
    }
    return EXIT_SUCCESS;
}