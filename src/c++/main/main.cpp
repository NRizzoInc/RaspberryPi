// Standard Includes
#include <iostream>
#include <csignal> // for ctrl+c signal handling
#include <thread>  // TODO: remove after web app self manages thread
#include <vector>  // TODO: remove after web app self manages thread

// 3rd Party Includes

// Our Includes
#include "constants.h"
#include "string_helpers.hpp"
#include "GPIO_Controller.h"
#include "CLI_Parser.h"
#include "server.h"
#include "client.h"
#include "tcp_base.h"
#include "backend.h"

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
    const int net_port {std::stoi(parse_res[RPI::CLI::Results::NET_PORT])}; // dont convert this twice
    static std::shared_ptr<RPI::Network::TcpBase> net_agent {
        is_client ?
          (RPI::Network::TcpBase*) new RPI::Network::TcpClient{parse_res[RPI::CLI::Results::IP], net_port, is_client} :
          (RPI::Network::TcpBase*) new RPI::Network::TcpServer{net_port, !is_client}
    };

    // Create UI Event Listener to interact with client
    static RPI::UI::WebApp net_ui{net_agent, std::stoi(parse_res[RPI::CLI::Results::WEB_PORT])};

    /* ========================================= Create Ctrl+C Handler ======================================== */
    // setup ctrl+c handler w/ callback to stop threads
    // store handler to chain with other potential handlers due to other libraries (ahem... crow)
    // using their own signal handler that would overwrite mine
    // reference: https://stackoverflow.com/a/10701909/13933174
    std::signal(SIGINT, [](int signum) {
        cout << "Caught ctrl+c: " << signum << endl;
        if(gpio_handler.setShouldThreadExit(true) != RPI::ReturnCodes::Success) {
            cerr << "Error: Failed to stop gpio thread" << endl;
        }
        
        if(net_agent->setExitCode(true) != RPI::ReturnCodes::Success) {
            cerr << "Error: Failed to stop network thread" << endl;
        }

        net_ui.stopWebApp();
    });

    /* ========================================== Initialize & Start ========================================= */

    // TODO: Remove and replace with RAII in classes
    std::vector<std::thread> thread_list;

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

    } else {
        // is client (startup web app interface for receiving commands)
        thread_list.push_back(std::thread{
            [&](){
                net_ui.startWebApp();
            }
        });
    }

    // startup client or server in a thread
    if (is_net) {
        cout << "Started net agent" << endl;
        net_agent->runNetAgent(false);
    }

    /* =============================================== Cleanup =============================================== */
    // make sure signal handler is up to date

    if(net_agent->cleanup() != RPI::ReturnCodes::Success) {
        const std::string net_agent_name {is_client ? "client" : "server"};
        cerr << "Failed to cleanup " << net_agent_name << " " << endl;
        return EXIT_FAILURE;
    }
    if(gpio_handler.cleanup() != RPI::ReturnCodes::Success) {
        cerr << "Failed to cleanup gpio" << endl;
        return EXIT_FAILURE;
    }

    // TODO: Remove 
    for (auto& proc : thread_list) {
        if (proc.joinable()) {
            proc.join();
        }
    }

    return EXIT_SUCCESS;
}