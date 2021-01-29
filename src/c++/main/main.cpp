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
#include "tcp_server.h"
#include "tcp_client.h"
#include "tcp_base.h"
#include "backend.h"
#include "rpi_camera.h"
#include "version.h"

using std::cout;
using std::cerr;
using std::endl;

int main(int argc, char* argv[]) {
    /* ============================================ Parse CLI Flags =========================================== */
    // object that parses the command line inputs
    RPI::gpio::CLI_Parser cli_parser(
        argc,
        argv,
        RPI::gpio::GPIOController::getModes(),
        RPI::gpio::GPIOController::getLedColorList(),
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

    /* ================================== Preform Preliminary (low cost) Work ================================= */

    // get results
    const bool show_version      { Helpers::toBool(parse_res[RPI::CLI::Results::ParseKeys::VERSION]) };
    static const bool is_verbose { Helpers::toBool(parse_res[RPI::CLI::Results::ParseKeys::VERBOSITY]) };

    // determine if dealing with server or client
    // need to be static to be captured by ctrl+c lambda
    static const bool is_client { parse_res[RPI::CLI::Results::ParseKeys::MODE] == "client" };
    static const bool is_server { parse_res[RPI::CLI::Results::ParseKeys::MODE] == "server" };
    static const bool is_cam    { parse_res[RPI::CLI::Results::ParseKeys::MODE] == "camera" };
    static const bool is_net    { is_client || is_server }; // true if client or server

    if(show_version || is_verbose) {
        cout << "Git Build SHA1: "      << RPI::Version::GIT_SHA1 << endl;
        cout << "Git Build Branch: "    << RPI::Version::GIT_BRANCH << endl;
        cout << "Git Commit Subject: "  << RPI::Version::GIT_COMMIT_SUBJECT << endl;
        cout << "Git Commit Date: "     << RPI::Version::GIT_DATE << endl;
        cout << "Git Describe: "        << RPI::Version::GIT_DESCRIBE << endl;
        if (show_version) return EXIT_SUCCESS; // exit if just showing version
    }

    /* ============================================ Create GPIO Obj =========================================== */
    // create single static gpio obj to controll rpi
    // static needed so it can be accessed in ctrl+c lambda
    static RPI::gpio::GPIOController gpio_handler{
        // motor i2c addr (convert hex string to int using base 16)
        static_cast<std::uint8_t>(std::stoi(parse_res[RPI::CLI::Results::ParseKeys::I2C_ADDR], 0, 16)),
        is_verbose
    };

    /* ======================================== Create Server OR Client ======================================= */
    // static needed so it can be accessed in ctrl+c lambda
    // don't convert string port numbers to ints twice
    const int ctrl_port {std::stoi(parse_res[RPI::CLI::Results::ParseKeys::CTRL_PORT])};
    const int cam_port  {std::stoi(parse_res[RPI::CLI::Results::ParseKeys::CAM_PORT])};
    static std::shared_ptr<RPI::Network::TcpBase> net_agent {
        is_client ?
            (RPI::Network::TcpBase*) new RPI::Network::TcpClient{
                parse_res[RPI::CLI::Results::ParseKeys::IP],
                ctrl_port,
                cam_port,
                is_client,
                is_verbose
            } 
            :
            (RPI::Network::TcpBase*) new RPI::Network::TcpServer{
                ctrl_port,
                cam_port,
                is_server,
                is_verbose
            }
    };

    // Create UI Event Listener to interact with client
    static RPI::UI::WebApp net_ui{net_agent, std::stoi(parse_res[RPI::CLI::Results::ParseKeys::WEB_PORT])};

    /* ======================================== Create Server OR Client ======================================= */

    // TODO: remove & add to server
    const int max_frames {std::stoi(parse_res[RPI::CLI::Results::ParseKeys::VID_FRAMES])};
    // only setup camera if available from server or camera test code
    const bool should_init_cam { is_cam || is_server };
    static RPI::Camera::CamHandler Camera{
        is_verbose,
        max_frames,
        should_init_cam,
        parse_res[RPI::CLI::Results::ParseKeys::FACEXML],
        parse_res[RPI::CLI::Results::ParseKeys::EYEXML]
    };


    /* ========================================= Create Ctrl+C Handler ======================================== */
    // setup ctrl+c handler w/ callback to stop threads
    std::signal(SIGINT, [](int signum) {
        cout << "Caught ctrl+c: " << signum << endl;
        if(gpio_handler.setShouldThreadExit(true) != RPI::ReturnCodes::Success) {
            cerr << "Error: Failed to stop gpio thread" << endl;
        }

        if(net_agent->sendResetPkt() != RPI::ReturnCodes::Success) {
            cerr << "Error: Failed to send reset command" << endl;
        }
        
        if(net_agent->setExitCode(true) != RPI::ReturnCodes::Success) {
            cerr << "Error: Failed to stop network thread" << endl;
        }

        if(Camera.setShouldStop(true) != RPI::ReturnCodes::Success) {
            cerr << "Error: Failed to stop camera thread" << endl;
        }

        if(net_ui.stopWebApp() != RPI::ReturnCodes::Success) {
            cerr << "Error: Failed to stop web app" << endl;
        }
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
            bool rtn_code {true};
            rtn_code &= gpio_handler.gpioHandlePkt(pkt) == RPI::ReturnCodes::Success;
            rtn_code &= Camera.setShouldRecord(pkt.cntrl.camera.is_on) == RPI::ReturnCodes::Success;
            return rtn_code ? RPI::ReturnCodes::Success : RPI::ReturnCodes::Error;
        });

        if(Camera.setGrabCallback([&](const std::vector<unsigned char>& grabbed_frame) {
                net_agent->setLatestCamFrame(grabbed_frame);
            }
        ) != RPI::ReturnCodes::Success) {
            cerr << "Error: Failed to set camera grab callback" << endl;
        }

    } else {
        // is client (startup web app interface for receiving commands)
        thread_list.push_back(std::thread{
            [&](){
                net_ui.startWebApp();
            }
        });
    }

    // need to start camera if testing camera or running server
    if (is_cam || is_server) {
        thread_list.push_back(std::thread{
            [&](){
                // only save frames to disk if running camera test
                Camera.RunFrameGrabber(true, is_cam);
            }
        });
    }

    // startup client or server in a thread
    if (is_net) {
        net_agent->runNetAgent(is_verbose);
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