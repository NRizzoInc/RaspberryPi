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
    // http://wiringpi.com/reference/software-pwm-library/
    constexpr int LED_SOFT_PWM_MIN      {0};
    constexpr int LED_SOFT_PWM_MAX      {100};
    constexpr int LED_SOFT_PWM_RANGE    {LED_SOFT_PWM_MAX - LED_SOFT_PWM_MIN};

    namespace Network {
        constexpr std::size_t   MAX_DATA_SIZE   {512};
        constexpr char          PKT_ACK[]       {"Packet ACK\n"};
        constexpr int           RECV_TIMEOUT    {5}; // TODO: heartbeat keepalive (shorten)
        constexpr int           ACPT_TIMEOUT    {5}; // ctrl+c takes 5 sec to work pre-connect
    } // end of Network namespace

    namespace Camera {
        constexpr int           VID_WIDTH       {640};
        constexpr int           VID_HEIGHT      {480};
        constexpr int           VID_FRAMERATE   {10}; // TODO: upgrade to 25 if 10 is okay

    }; //end of camera namespace

}; // end of constants namespace

namespace CLI::Results {
    // shortening of parse results mapping
    using ParseResults = std::unordered_map<std::string, std::string>;

    // cannot use std::string in constexpr (auto == char[])
    constexpr auto MODE             = "mode"       ;
    constexpr auto COLORS           = "names"      ;
    constexpr auto INTERVAL         = "interval"   ;
    constexpr auto RATE             = "rate"       ;
    constexpr auto DURATION         = "duration"   ;
    constexpr auto IP               = "ip"         ;
    constexpr auto NET_PORT         = "net-port"   ;
    constexpr auto WEB_PORT         = "web-port"   ;
    constexpr auto MOTOR_ADDR       = "motor-addr" ;
    constexpr auto VID_FRAMES       = "frames" ;
}; // end of CLI::Results namespace

}; // end of RPI namespace

#endif