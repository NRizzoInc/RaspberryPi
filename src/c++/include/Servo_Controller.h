#ifndef SERVO_CONTROLLER_H
#define SERVO_CONTROLLER_H

// Standard Includes
#include <iostream>
#include <string>
#include <vector>
#include <cmath>        // for abs
#include <algorithm>    // for max/min
#include <chrono>       // for setting sleep durations
#include <thread>       // for std::this_thread
#include <optional>
#include <unordered_map>

// Our Includes
#include "constants.h"
#include "PCA9685_Interface.h"
#include "timing.hpp"

// 3rd Party Includes

namespace RPI {
namespace gpio {
namespace Servo {

// commonly used in this namespace
using Interface::XDirection;
using Interface::YDirection;

// no servo can ever go passed these values
constexpr int ANGLE_ABS_MIN     {0};
constexpr int ANGLE_ABS_MAX     {180};
constexpr int ANGLE_ABS_RANGE   {ANGLE_ABS_MAX-ANGLE_ABS_MIN};

// servo movements are all based on the duty cycle (percentage of time pulse is on/off)
// ex: assume freq = 50Hz (aka period = 20ms)
// 0°   : 0.5 ms duty period ( 2.5%  duty)
// 90°  : 1.5 ms duty period ( 7.5%  duty)
// 180° : 2.5 ms duty period (12.5%  duty)
// hence, scale between min-max duties based on angle
constexpr float DUTY_PERC_MIN   {.025}; // 2.5%
constexpr float DUTY_PERC_MAX   {.125}; // 12.5%
constexpr float DUTY_PERC_RANGE {DUTY_PERC_MAX-DUTY_PERC_MIN};

/**
 * Servos might be limited by physical/hardware such that the normal 0-180 rotation is actually askew.
 * This struct is used to define the max/min angle a servo can take so user does not have to worry about these details
 * In essence, maps normal [0, 180] => [Actual min, actual max]
 */
struct ServoLimits {
    /**
     * @param min The min possible angle
     * @param max The max possible angle
     * @param opp_dir (default=false) true if servo moves opposite expected and direction of travel should be flipped
     * (usually a hardware limitation where 0° & 180° are opposite of what you want them to be)
     */
    ServoLimits(const int min=ANGLE_ABS_MIN, const int max=ANGLE_ABS_MAX, const bool opp_dir=false)
        : min{min}
        , max{max}
        , range{max-min}
        , opp{opp_dir ? -1 : 1} // 1 (positive) means normal 
        {}

    const int min;      // the min angle the servo can take (maps to 0°)
    const int max;      // the max angle the servo can take (maps to 180°)
    const int range;    // the servo's actual arc length (i.e. 90°-180° would have range = 90°)
    const int opp;     // (1=normal) -1 if this servo's 0° produces what 180° should and vice versa
}; // end of ServoLimits

/**
 * @brief Contains all relevant data for a servo
 */
struct ServoData {
    /**
     * @param angle_limits The limits of the given servo
     * @param start_angle The position the servo should start at
     */
    ServoData(const ServoLimits& angle_limits=ServoLimits{}, const int start_angle=90)
        : limits{angle_limits}
        , pos{start_angle}
        {}

    const ServoLimits limits;  // the servo's movement/angle limits
    int pos;                        // the current angle position of the servo
}; // end of ServoData

// Maps each tire/motor/servo to its i2c address
// note robot has 8 total possible servo slots (but only 2 are used) 
enum class I2C_ServoAddr : int {
    YAW         =  8,         // Controls sideways camera servo
    PITCH       =  9,         // Controls vertical camera servo
    UNUSED_1    = 10,
    UNUSED_2    = 11,
    UNUSED_3    = 12,
    UNUSED_4    = 13,
    UNUSED_5    = 14,
    UNUSED_6    = 15,
}; // end of servo addresses

// contains the address (aka the servo to move) and the angle to move it to
struct ServoAnglePair {
    /**
     * @param sel_servo The selected servo
     * @param angle the angle to move the selected servo
     */
    ServoAnglePair(const I2C_ServoAddr sel_servo, const std::optional<int> angle=std::nullopt)
        : sel_servo{sel_servo}
        , angle{angle}
        {}
    const I2C_ServoAddr sel_servo;
    const std::optional<int> angle;
}; // end of ServoAnglePair

/**
 * Handle class for I2C Chip for servos (PCA9685)
 * 
 * see https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf
 * (slightly different bc using "off" period and not "on" period -- +/-2.5% to extremes)
 * Servo pwm math based on duty cycle percentages:
 * 0°   :  2.5% (normally 5%)
 * 90°  :  7.5%
 * 180° : 12.5% (normally 10%)
 * 360° continous spinning: >= ~13%
 */
class ServoController : public gpio::Interface::PCA9685 {

    public:
        /********************************************** Constructors **********************************************/

        /**
         * @brief Construct a servo controller object responsible for handling servo interactions
         * @param servo_i2c_addr The address of the servo controller i2c board
         * @param verbosity If true, will print more information that is strictly necessary
         */
        ServoController(const std::uint8_t servo_i2c_addr, const bool verbosity=false);
        virtual ~ServoController();

        /**
         * @brief Helps intialize the servos
         * @return ReturnCodes
         */
        virtual ReturnCodes init() const;

        /********************************************* Getters/Setters *********************************************/

        /**
         * @brief Gets the current position of the selected servo
         * @param sel_servo The servo whose position to check
         * @return The servo's current angle
         * 0°: negative (left/down)
         * 90°: neutral
         * 180°: positive (right/up)
         */
        int GetServoPos(const I2C_ServoAddr sel_servo) const;

        /**
         * @brief Get the servo's angle limits
         * @param sel_servo The servo whose limits to check
         * @return The servo's angle limits
         */
        const ServoLimits& GetServoLimits(const I2C_ServoAddr sel_servo) const;


        /********************************************* Servo Functions *********************************************/

        /**
         * @brief Changes a single servo's position by a descrete angle (final endpoint for overloads)
         * @param sel_servo The specific servo to set/move
         * @param angle The amount to change the angle by [-180, 180]
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes IncrementServoPos(const I2C_ServoAddr sel_servo, const int change_amt) const;
        ReturnCodes IncrementServoPos(const ServoAnglePair pair) const;
        ReturnCodes IncrementServoPos(const std::vector<ServoAnglePair> servo_angle_pairs) const;

        /**
         * @brief Gradually move a servo from start_angle -> end_angle
         * @param sel_servo The servo to gradually move
         * @param duration How long the movement of the servo should take
         * @param end_angle The angle at which the servo should stop at
         * @param start_pos The starting position of the servo (defaults to current position)
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes GradualMoveServo(
            const I2C_ServoAddr sel_servo,
            const std::chrono::steady_clock::duration duration,
            const int end_angle,
            const std::optional<int> start_angle=std::nullopt
        ) const;

        /**
         * @brief Set a single servo's pwm with a desired duty cycle (final endpoint for overloads)
         * @param sel_servo The specific servo to set/move
         * @param angle The position to move the servo to (if none, moves servo to current position)
         * @return ReturnCodes Success if no issues
         * @note leave angle empty for initialization
         */
        ReturnCodes SetServoPos(const I2C_ServoAddr sel_servo, const std::optional<int> angle=std::nullopt) const;
        ReturnCodes SetServoPos(const ServoAnglePair) const;
    
        /**
         * @brief Sets all servos' pwm with a desired duty cycle
         * @param servo_angle_pairs vector pairs of servos & where to move them to
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes SetServoPos(const std::vector<ServoAnglePair> servo_angle_pairs) const;

        /**
         * @brief Responsible for turning off the servos so they do not stay primed past the duration of the program
         * @return ReturnCodes Success if no issues
         */
        ReturnCodes TurnServosOff() const;

        /**
         * @brief Run the servos through a set pattern or have them move to a specific location and stop
         * @note Have to pass everything by reference do to function mapping requirements
         */
        void testServos(
            // not needed, but need to follow call guidlines for fn-mapping to work
            __attribute__((unused)) const std::vector<std::string>& colors={},
            const unsigned int& interval=1000,
            const int& duration=-1,
            __attribute__((unused)) const unsigned int& rate=1
        ) const;

    private:
        /******************************************** Private Variables ********************************************/

        static std::unordered_map<I2C_ServoAddr, ServoData> servos; // maps servos' to their current positions (angles)

        /********************************************* Helper Functions ********************************************/

        /**
         * @brief Convert the easily understandable angle to the servo's corresponding pwm pulse value
         * @param sel_servo The servo whose pwm duty signal you want to find
         * @param angle The angle to check if is valid and covnert to pwm value for the servo [0-180]
         * @return Integer representing the pwm "off" period value to use
         */
        int AngleToPwmPulse(const I2C_ServoAddr sel_servo, const int angle) const;

        /**
         * @brief Makes sure the passed angle is within the valid range
         * @param sel_servo The servo whose angle is being validated
         * @param angle The angle to validate/check
         * @return An angle within the range [0, 180]
         * @note Should only be used by SetServoPos() or else might counteract itself
         */
        int ValidateAngle(const I2C_ServoAddr sel_servo, const int angle) const;

}; // ServoController

}; // end of Servo namespace
}; // end of gpio namespace
}; // end of RPI namespace


#endif
