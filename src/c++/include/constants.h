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
};

namespace Constants {

    namespace GPIO {
        // http://wiringpi.com/reference/software-pwm-library/
        constexpr int LED_SOFT_PWM_MIN      {0};
        constexpr int LED_SOFT_PWM_MAX      {100};
        constexpr int LED_SOFT_PWM_RANGE    {LED_SOFT_PWM_MAX - LED_SOFT_PWM_MIN};
    }; // end of Constants::GPIO namespace

    namespace Network {
        constexpr std::size_t   MAX_DATA_SIZE   {512};
        constexpr char          PKT_ACK[]       {"Packet ACK\n"};
        constexpr int           RECV_TIMEOUT    {5}; // TODO: heartbeat keepalive (shorten)
        constexpr int           ACPT_TIMEOUT    {5}; // ctrl+c takes 5 sec to work pre-connect
    } // end of Network namespace

    namespace Camera {
        constexpr int           FRAME_WIDTH     {640};
        constexpr int           FRAME_HEIGHT    {480};
        constexpr int           FRAME_SIZE      {FRAME_WIDTH*FRAME_HEIGHT};
        constexpr int           VID_FRAMERATE   {10}; // TODO: upgrade to 25 if 10 is okay

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
        MOTOR_ADDR,
        VID_FRAMES
    }; // end of ParseResults's keys

    // maps CLI's results (stored in string form) as {ParseKeys::<key> : value}
    using ParseResults = std::unordered_map<ParseKeys, std::string>;
}; // end of CLI::Results namespace

}; // end of RPI namespace

#endif