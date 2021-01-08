#include "Servo_Controller.h"

namespace RPI {
namespace gpio {
namespace Servo {

using std::cout;
using std::cerr;
using std::endl;

/********************************************* Static Defines *********************************************/

/// Maps a specific servo to its current (angle) position
///@note init servos to neutral position (aka 90 degrees)
std::unordered_map<I2C_ServoAddr, int> ServoController::pos{
    {I2C_ServoAddr::YAW, 90},
    {I2C_ServoAddr::PITCH, 90}
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

    // starts servos at 90 degrees (neutral)
    if (SetServoPos({{I2C_ServoAddr::PITCH, 90}, {I2C_ServoAddr::YAW, 90}}) != ReturnCodes::Success) {
        cerr << "Error: Failed to init servos to neutral position" << endl;
    }

    setIsInit(true);
    return ReturnCodes::Success;
}


/********************************************* Getters/Setters *********************************************/

int ServoController::GetServoPos(const I2C_ServoAddr sel_servo) const {
    return pos.at(sel_servo);
}


/********************************************* Servo Functions *********************************************/

ReturnCodes ServoController::IncrementServoPos(const I2C_ServoAddr sel_servo, const int change_amt) const {
    const int updated_angle       {GetServoPos(sel_servo) + change_amt};
    const int valid_updated_angle {ValidateAngle(updated_angle)};
    return SetServoPos(sel_servo, valid_updated_angle);
}

ReturnCodes ServoController::IncrementServoPos(const ServoAnglePair pair) const {
    return IncrementServoPos(pair.sel_servo, pair.angle);
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
    const int start_pos             { start_angle ? *start_angle : GetServoPos(sel_servo) };
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
            !ServoController::getShouldThreadExit() ||
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

ReturnCodes ServoController::SetServoPos(const I2C_ServoAddr sel_servo, const int angle) const {
    // try to convert angle to pwm signal (if success, udpate current state)
    ReturnCodes rtn = SetPwm(static_cast<int>(sel_servo), 0, AngleToPwmDuty(angle));
    if (rtn == ReturnCodes::Success) {
        pos[sel_servo] = angle;
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
    auto MoveServo = [&](bool is_yaw, int end_angle) {
        return GradualMoveServo(
            is_yaw ? I2C_ServoAddr::YAW : I2C_ServoAddr::PITCH,
            std::chrono::milliseconds(interval),
            end_angle
        );
    };

    // helps keep track if duration is up (needed bc loop may take awhilem but can be split & stopped in piecemeal)
    auto isDurationUp = [&]()->bool {
        // if duration == -1 : run forever
        return
            duration != -1 &&
            Helpers::Timing::hasTimeElapsed(start_time, duration, std::chrono::milliseconds(1));
    };

    while (!ServoController::getShouldThreadExit() && !isDurationUp()) {
        // sweep neutral->right
        cout << "Sweeping Servo Right" << endl;
        if (MoveServo(true, 180) != ReturnCodes::Success) {
            cerr << "Error: Failed to sweep servo right" << endl;
        }
        if (ServoController::getShouldThreadExit() || isDurationUp()) break;

        // sweep right->left

        // neutral

        // sweep up

        // sweep down

        // neutral
    }
}


/********************************************* Helper Functions ********************************************/

float ServoController::ScaleAnglePercDuty(const int angle) const {
    // scale [0-180] -> [1.0-2.0]
    // make angle stay between 0-180
    // ex: assume freq = 50Hz (aka period = 20ms)
    // 0   degrees: 1.0 ms duty period (5%   duty)
    // 90  degrees: 1.5 ms duty period (7.5% duty)
    // 180 degrees: 2.0 ms duty period (10%  duty)
    // hence, min=5%, max=10% and scale between them
    constexpr float DUTY_MIN_PERC       {.05}; // 5%
    constexpr float DUTY_MAX_PERC       {.10}; // 10%
    constexpr float DUTY_RANGE          {DUTY_MAX_PERC-DUTY_MIN_PERC};

    // perform calcs to scale angle to % duty cycle
    // multiply percent against the actual period to get the final answer
    const int   valid_angle {ValidateAngle(angle)};
    const float perc_angle  {static_cast<float>(valid_angle) / static_cast<float>(ANGLE_MAX)};
    const float perc_duty   {DUTY_RANGE*perc_angle + DUTY_MIN_PERC};
    return perc_duty;
}


int ServoController::AngleToPwmDuty(const int angle) const {
    // whatever duty cycle/period/ticks comes out to be, have to scale it with the max allowed PWM signal
    // https://learn.adafruit.com/adafruits-raspberry-pi-lesson-8-using-a-servo-motor/servo-motors
    // https://cdn-shop.adafruit.com/datasheets/PCA9685.pdf -- [age 25]

    // get the duty cycle (max possible pwm * percentage on)
    const float perc_duty {ScaleAnglePercDuty(angle)};
    return PCA9685::MAX_PWM * perc_duty;
}

int ServoController::ValidateAngle(const int angle) const {
    // between 0-180
    return std::max(ANGLE_MIN, std::min(ANGLE_MAX, angle));
}



}; // end of Servo namespace
}; // end of gpio namespace
}; // end of RPI namespace
