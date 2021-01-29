#ifndef CONSTANTS_H
#define CONSTANTS_H

#include <string>
#include <unordered_map>

namespace RPI {
// Defines different possible returns rather than just success/fail
enum class ReturnCodes {
    Success,
    Error,
    TryAgain,
    Timeout
};

namespace Constants {

    namespace GPIO {
        // http://wiringpi.com/reference/software-pwm-library/
        constexpr int LED_SOFT_PWM_MIN      {0};
        constexpr int LED_SOFT_PWM_MAX      {100};
        constexpr int LED_SOFT_PWM_RANGE    {LED_SOFT_PWM_MAX - LED_SOFT_PWM_MIN};
    }; // end of Constants::GPIO namespace

    namespace Network {
        constexpr std::size_t   MAX_DATA_SIZE   {4096};
        constexpr char          PKT_ACK[]       {"Packet ACK\n"};
        constexpr int           RX_TX_TIMEOUT   {1}; // heartbeat (ctrl+c takes this long during runtime)
        constexpr int           ACPT_TIMEOUT    {2}; // ctrl+c takes this long to work pre-connect
    } // end of Network namespace

    namespace Camera {
        constexpr int           FRAME_WIDTH     {640};
        constexpr int           FRAME_HEIGHT    {480};
        constexpr int           FRAME_SIZE      {FRAME_WIDTH*FRAME_HEIGHT};
        constexpr int           VID_FRAMERATE   {25};

    }; //end of camera namespace

}; // end of constants namespace

namespace CLI::Results {

    // keys for the mapping of the CLI results (stored in ParseResults)
    enum class ParseKeys {
        MODE,
        COLORS,
        INTERVAL,
        RATE,
        DURATION,
        IP,
        CTRL_PORT,
        CAM_PORT,
        WEB_PORT,
        I2C_ADDR,
        VID_FRAMES,
        FACEXML,
        EYEXML,
        VERBOSITY,
        VERSION
    }; // end of ParseResults's keys

    // maps CLI's results (stored in string form) as {ParseKeys::<key> : value}
    using ParseResults = std::unordered_map<ParseKeys, std::string>;
}; // end of CLI::Results namespace

}; // end of RPI namespace

#endif