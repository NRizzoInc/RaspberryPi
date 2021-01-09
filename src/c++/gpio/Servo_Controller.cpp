#include "Servo_Controller.h"

namespace RPI {
namespace gpio {
namespace Servo {

using std::cout;
using std::cerr;
using std::endl;

/********************************************* Static Defines *********************************************/

/// Maps a specific servo to its current (angle) position
/// init servos to center position (aka 90°)
/// Yaw (sideways) servo - Normal
/// Pitch (vertical) servo is reversed (placed at center is actually 270) & cannot go lower than 90 bc hardware
std::unordered_map<I2C_ServoAddr, ServoData> ServoController::servos{
    std::pair{
        I2C_ServoAddr::YAW,
        ServoData{ServoAngleLimits{0, 180}, 90}
    },
    std::pair{
        I2C_ServoAddr::PITCH,
        ServoData{ServoAngleLimits{90, 180}, 90}
    }
};

/********************************************** Constructors **********************************************/


ServoController::ServoController(const std::uint8_t servo_i2c_addr)
    : PCA9685{servo_i2c_addr}
{
    // stub
}

ServoController::~ServoController() {
    // only cleanup if is init in first place
    // prevents client from trying to write to registers that do not exist
    if (ServoController::getIsInit()) {
        // make motors stop
        cout << "Resetting Servo Pins" << endl;
        // todo: convert this to single function
        if (TurnServosOff() != ReturnCodes::Success) {
            cerr << "Error: Failed to turn off servos" << endl;
        }
        cleanup();
    }
    setIsInit(false);
}

ReturnCodes ServoController::init() const {
    // if already init, stop now (have to specify whose getIsInit to call otherwise always true)
    if (ServoController::getIsInit()) return ReturnCodes::Success;

    if (PCA9685::init() != ReturnCodes::Success) {
        return ReturnCodes::Error;
    }

    // starts servos at 90° (center)
    if (SetServoPos({{I2C_ServoAddr::PITCH}, {I2C_ServoAddr::YAW}}) != ReturnCodes::Success) {
        cerr << "Error: Failed to init servos to center position" << endl;
    }

    setIsInit(true);
    return ReturnCodes::Success;
}


/********************************************* Getters/Setters *********************************************/

int ServoController::GetServoPos(const I2C_ServoAddr sel_servo) const {
    return servos.at(sel_servo).pos;
}

const ServoAngleLimits& ServoController::GetServoLimits(const I2C_ServoAddr sel_servo) const {
    return servos.at(sel_servo).limits;
}


/********************************************* Servo Functions *********************************************/

ReturnCodes ServoController::IncrementServoPos(const I2C_ServoAddr sel_servo, const int change_amt) const {
    const int updated_angle       {GetServoPos(sel_servo) + change_amt};
    const int valid_updated_angle {ValidateAngle(sel_servo, updated_angle)};
    return SetServoPos(sel_servo, valid_updated_angle);
}

ReturnCodes ServoController::IncrementServoPos(const ServoAnglePair pair) const {
    return IncrementServoPos(
        pair.sel_servo,
        // if no angle provided, default to not moving (change of 0)
        pair.angle ? *pair.angle : 0
    );
}

ReturnCodes ServoController::IncrementServoPos(const std::vector<ServoAnglePair> servo_angle_pairs) const {
    for (const auto& pair : servo_angle_pairs) {
        if (IncrementServoPos(pair) != ReturnCodes::Success) {
            return ReturnCodes::Error;
        }
    }
    return ReturnCodes::Success;
}


ReturnCodes ServoController::GradualMoveServo(
    const I2C_ServoAddr sel_servo,
    const std::chrono::steady_clock::duration duration,
    const int end_angle,
    const std::optional<int> start_angle
) const {
    // factor in start angle not being current position
    const int start_pos             { start_angle.has_value() ? *start_angle : GetServoPos(sel_servo) };
    const int sweep_displacement    { end_angle - start_pos };

    // unit of time per each angle (handle divide by zero)
    // note: std::chrono::steady_clock::duration stores as ns
    // hence, rate = ns / angle
    const std::chrono::steady_clock::duration rate { 
        sweep_displacement != 0 ?
            abs(duration / sweep_displacement)
            : std::chrono::steady_clock::duration(0)
    };

    // fill vector with range of values from start to end
    // std::vector<int> positions(sweep_displacement);
    // std::iota(std::begin(positions), std::end(positions), start_pos); // each element is +1
    // cout << Helpers::createVecStr(positions) << endl;

    // two completely different stop conditions & increment/decrement based on if displacement is negative
    const bool is_neg { sweep_displacement < 0};
    auto isEnd = [&](const int pos_to_check){
        return
            // this function may take awhile, so be mindful if told to stop
            ServoController::getShouldThreadExit() ||
            is_neg ?
                pos_to_check < end_angle : // if negative, going towards lower angles (stop when less than end)
                pos_to_check > end_angle ; // if positive, going towards higher angles (stop when higher than end)
    };

    // if negative, decrement (i.e. subtract by 1)
    for (int curr_pos = start_pos; !isEnd(curr_pos); curr_pos += is_neg ? -1 : 1 ) {
        if(SetServoPos(sel_servo, curr_pos) != ReturnCodes::Success) {
            cerr << "Error: Failed to gradually move servo" << endl;
            return ReturnCodes::Error;
        }
        std::this_thread::sleep_for(rate);
    }

    // completed movement
    return ReturnCodes::Success;
}

ReturnCodes ServoController::SetServoPos(
    const I2C_ServoAddr sel_servo,
    const std::optional<int> angle
) const {
    // try to convert angle to pwm signal
    // if no angle provided, default to current position
    const int real_angle {angle ? *angle : GetServoPos(sel_servo)};
    ReturnCodes rtn = SetPwm(
        static_cast<int>(sel_servo),
        0,
        AngleToPwmPulse(sel_servo, real_angle)
    );

    // (if success, update current state)
    if (rtn == ReturnCodes::Success) {
        servos[sel_servo].pos = real_angle;
    }
    return rtn;
}

ReturnCodes ServoController::SetServoPos(const ServoAnglePair pair) const {
    return SetServoPos(pair.sel_servo, pair.angle);
}

ReturnCodes ServoController::SetServoPos(const std::vector<ServoAnglePair> servo_angle_pairs) const {
    for (const auto& pair : servo_angle_pairs) {
        if (SetServoPos(pair) != ReturnCodes::Success) {
            return ReturnCodes::Error;
        }
    }
    return ReturnCodes::Success;
}

ReturnCodes ServoController::TurnServosOff() const {
    // have to make sure to turn ON mode off, and OFF mode on
    bool rtn_succ {true};
    rtn_succ &= TurnFullOff(static_cast<int>(I2C_ServoAddr::PITCH), true) == ReturnCodes::Success;
    rtn_succ &= TurnFullOn(static_cast<int>(I2C_ServoAddr::PITCH), false) == ReturnCodes::Success;
    rtn_succ &= TurnFullOff(static_cast<int>(I2C_ServoAddr::YAW), true) == ReturnCodes::Success;
    rtn_succ &= TurnFullOn(static_cast<int>(I2C_ServoAddr::YAW), false) == ReturnCodes::Success;
    return rtn_succ ? ReturnCodes::Success : ReturnCodes::Error;
}

void ServoController::testServos(
    // not needed, but need to follow call guidlines for fn-mapping to work
    __attribute__((unused)) const std::vector<std::string>& colors,
    const unsigned int& interval,
    const int& duration,
    __attribute__((unused)) const unsigned int& rate
) const {
    cout << "Interval: " << interval << "ms" << endl;
    cout << "Duration: " << duration << "ms" << endl;

    // keep track of time/duration
    const auto start_time = std::chrono::steady_clock::now();

    // helper fn that simplifies this repetitive function call
    /// @param is_yaw true if sideways camera, false if vertical camera
    /// @param end_angle the ending position of the servo
    /// @param dir_travel To the direction of travel (i.e. "left->right")
    /// @param dist_pen How fast -- needed bc delta=180 will be super fast compared to delta=90
    /// (i.e.: 1x or 2x if normal or double distance)
    auto MoveServo = [&](bool is_yaw, int end_angle, const std::string& dir_travel, int dist_pen=1) {
        cout << "Sweeping Servo " << dir_travel << endl;
        if(GradualMoveServo(
            is_yaw ? I2C_ServoAddr::YAW : I2C_ServoAddr::PITCH,
            std::chrono::milliseconds(interval*dist_pen),
            end_angle
        ) != ReturnCodes::Success) {
            cerr << "Error: Failed to sweep servo " << dir_travel << endl;
        }
    };

    // helps keep track if duration is up (needed bc loop may take awhilem but can be split & stopped in piecemeal)
    auto isDurationUp = [&]()->bool {
        // if duration == -1 : run forever
        return
            duration != -1 &&
            Helpers::Timing::hasTimeElapsed(start_time, duration, std::chrono::milliseconds(1));
    };

    while (!ServoController::getShouldThreadExit() && !isDurationUp()) {
        ///////// horizontal

        MoveServo(true, 180, "Center -> Right", 1);
        if (ServoController::getShouldThreadExit() || isDurationUp()) break;

        // sweep 180->0 (double distance)
        MoveServo(true, 0, "Right -> Left", 2);
        if (ServoController::getShouldThreadExit() || isDurationUp()) break;

        MoveServo(true, 90, "Left -> Center", 1);
        if (ServoController::getShouldThreadExit() || isDurationUp()) break;

        ///////// vertical

        MoveServo(false, 180, "Center -> Up", 1);
        if (ServoController::getShouldThreadExit() || isDurationUp()) break;

        // sweep 180->0 (double distance)
        MoveServo(false, 0, "Up -> Down", 2);
        if (ServoController::getShouldThreadExit() || isDurationUp()) break;

        MoveServo(false, 90, "Down -> Center", 1);
        if (ServoController::getShouldThreadExit() || isDurationUp()) break;
    }
}


/********************************************* Helper Functions ********************************************/

int ServoController::AngleToPwmPulse(const I2C_ServoAddr sel_servo, const int angle) const {
    // whatever duty cycle/period/ticks comes out to be, have to scale it with the max allowed PWM signal
    // https://learn.adafruit.com/adafruits-raspberry-pi-lesson-8-using-a-servo-motor/servo-motors
    // https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf -- [age 25]

    // perform calcs to scale angle to % duty cycle
    // multiply percent against the actual period to get the final answer
    const int       valid_angle     {ValidateAngle(sel_servo, angle)};
    const float     perc_angle      {static_cast<float>(valid_angle) / static_cast<float>(ANGLE_ABS_MAX)};
    const float     perc_duty       {DUTY_PERC_RANGE*perc_angle + DUTY_PERC_MIN};

    // get the duty cycle (max possible pwm * percentage on)
    return static_cast<int>(PCA9685::MAX_PWM * perc_duty);
}

int ServoController::ValidateAngle(const I2C_ServoAddr sel_servo, const int angle) const {
    // should be between 0-180 but due to safety reasons servos cant do full range
    // user expects 0-180 so if limited, need to scale appropriately
    // i.e. min = 90 & max = 180:
    //  Input Angle :  Output Angle
    //      0°      :       90°
    //      90°     :       135°
    //      180°    :       180°
    // https://stats.stackexchange.com/questions/281162/scale-a-number-between-a-range/281164
    const ServoAngleLimits limits   {GetServoLimits(sel_servo)};
    const int angle_rel_to_min      {angle - ANGLE_ABS_MIN};
    const float perc_limit          {static_cast<float>(angle_rel_to_min) / static_cast<float>(ANGLE_ABS_RANGE)};
    return (limits.range * perc_limit) + limits.min;
}



}; // end of Servo namespace
}; // end of gpio namespace
}; // end of RPI namespace
