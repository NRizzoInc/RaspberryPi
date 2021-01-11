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
    add_option("-m,--mode", cli_res[CLI::Results::ParseKeys::MODE])
        ->description("Which action to perform")
        ->required(true)
        ->check(::CLI::IsMember(mode_list))
        // make sure only 1 mode is ever taken
        ->expected(1)
        ->take_first()
        ;

    /******************************************** Testing Flags *******************************************/

    auto test_group = add_option_group("Testing");

    const char delim {','};
    test_group->add_option("-c,--colors", cli_res[CLI::Results::ParseKeys::COLORS])
        ->description("Which LEDs/Buttons to use (comma-seperated)")
        ->required(false)
        // method for taking in multiple args => str-represented vector
        ->expected(0, color_list.size())
        ->allow_extra_args() // allow mutliple inputs despite type=str
        ->delimiter(delim)
        ->check(::CLI::IsMember(color_list))
        ->join(delim)
        ;

    test_group->add_option("-i,--interval", cli_res[CLI::Results::ParseKeys::INTERVAL])
        ->description("The interval (in ms) between changing LEDs' states")
        ->required(false)
        ->default_val("1000")
        ;

    test_group->add_option("-d,--duration", cli_res[CLI::Results::ParseKeys::DURATION])
        ->description("How long the program should run (in ms)")
        ->required(false)
        ->default_val("-1")
        ;

    test_group->add_option("-r,--rate", cli_res[CLI::Results::ParseKeys::RATE])
        ->description("How fast the LEDs' intensity should change (1x, 2x, 3x...)")
        ->required(false)
        ->default_val("1")
        ;

    /**************************************** Networking Flags ****************************************/

    auto net_group = add_option_group("Network");

    // mark them both as needing mode_opt bc its results impact them
    net_group->add_option("-a,--ip", cli_res[CLI::Results::ParseKeys::IP])
        ->description("The server's ip address")
        ->required(false)
        ->default_val("127.0.0.1")
        ->check(::CLI::ValidIPV4)
        ;

    net_group->add_option("-p,--control-port", cli_res[CLI::Results::ParseKeys::CTRL_PORT])
        ->description("The server's/client's port number for controlling the robot's movement")
        ->required(false)
        ->default_val("55555")
        ->check(::CLI::Range(1024, 65535))
        ;

    net_group->add_option("--cam-port", cli_res[CLI::Results::ParseKeys::CAM_PORT])
        ->description("The server's/client's port number for sending & receiving camera data")
        ->required(false)
        ->default_val("55556")
        ->check(::CLI::Range(1024, 65535))
        ;

    /**************************************** I2C Address Flags ***************************************/

    auto hardware_group = add_option_group("Hardware");

    hardware_group->add_option("--i2c-addr", cli_res[CLI::Results::ParseKeys::I2C_ADDR])
        ->description("The PCA9685's (motor/servo controller) i2c address in hex (find with i2cdetect -y 1)")
        ->required(false)
        ->default_val("0x40")
        ->check(::CLI::Number)
        ;

    /****************************************** Web App Flags *****************************************/

    net_group->add_option("--web-port", cli_res[CLI::Results::ParseKeys::WEB_PORT])
        ->description("The web-app's port number")
        ->required(false)
        ->default_val("5001")
        ->check(::CLI::Range(1024, 65535))
        ;

    /****************************************** Camera Flags *****************************************/

    auto cam_group = add_option_group("Camera");
    cam_group->add_option("-f,--frames", cli_res[CLI::Results::ParseKeys::VID_FRAMES])
        ->description("The number of frames to capture before stopping the video (-1 = infinite)")
        ->required(false)
        ->default_val("-1")
        ;

    cam_group->add_option("--face-xml", cli_res[CLI::Results::ParseKeys::FACEXML])
        ->description("The absolute path to the opencv `haarcascade_frontalface.xml` to use for facial recognition")
        ->required(false)
        ->check(::CLI::ExistingFile)
        ;

    cam_group->add_option("--eye-xml", cli_res[CLI::Results::ParseKeys::EYEXML])
        ->description("The absolute path to the opencv `haarcascade_eye_tree_eyeglasses.xml` to use for occular recognition")
        ->required(false)
        ->check(::CLI::ExistingFile)
        ;

    /*************************************** Miscellaneous Flags *************************************/
    add_flag("-v,--verbose", cli_res[CLI::Results::ParseKeys::VERBOSITY])
        ->description("Use this flag to increase verbosity (more prints)")
        ->required(false)
        ->default_val(false)
        ;

    return ReturnCodes::Success;
}


}; // end of gpio namespace

}; // end of RPI namespace
