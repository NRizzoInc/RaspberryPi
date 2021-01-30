#include "GPIO_Controller.h"

using std::cout;
using std::cerr;
using std::endl;

namespace RPI {

namespace gpio {

/**************************************** Static Member Variables *****************************************/

// define the mode to function map
const ModeMap GPIOController::mode_to_action {GPIOController::createFnMap()};

/********************************************** Constructors **********************************************/
GPIOController::GPIOController(const std::uint8_t i2c_addr, const bool verbosity)
    // call constructors for parents
    : LED::LEDController(verbosity)
    , Button::ButtonController(verbosity)
    , Motor::MotorController(i2c_addr, verbosity)
    , Servo::ServoController(i2c_addr, verbosity)
    , Ultrasonic::DistSensor{verbosity}

    // init vars
    , run_thread{}
    , started_thread{false}
    , has_cleaned_up{false}
{
    // stub
}

GPIOController::~GPIOController() {
    // stub
}

ReturnCodes GPIOController::cleanup() {
    // dont double cleanup
    if (has_cleaned_up) return ReturnCodes::Success;

    // wait to block until a thread has been setup
    // otherwise thread is empty and joins immediately
    // but max out time to wait or else program gets blocked here if thread is never started
    std::unique_lock<std::mutex> lk{thread_mutex};
    thread_cv.wait_for(
        lk,
        std::chrono::milliseconds(200),
        [&](){ return started_thread.load(); }
    );

    // block until thread ends
    if (run_thread.joinable()) {
        run_thread.join();
    }

    has_cleaned_up = true;
    return ReturnCodes::Success;
}

/********************************************* Getters/Setters *********************************************/

std::vector<std::string> GPIOController::getModes() {
    return Helpers::Map::getMapKeys(mode_to_action);
}


bool GPIOController::getIsInit() const {
    return LEDController::getIsInit() 
        && ButtonController::getIsInit()
        && MotorController::getIsInit()
        && ServoController::getIsInit()
        && DistSensor::getIsInit()
        ;
}


/*********************************************** GPIO Helpers **********************************************/

ReturnCodes GPIOController::init() const {
    // immediately return if already init
    if (getIsInit()) return ReturnCodes::Success;

    // otherwise init sub-components
    bool rtn {true}; // set to false if error

    const bool led_rtn { LEDController::init() == ReturnCodes::Success };
    rtn &= led_rtn;
    if (!led_rtn) cerr << "Failed to properly init LEDs" << endl;

    const bool btn_rtn { ButtonController::init() == ReturnCodes::Success };
    rtn &= btn_rtn;
    if (!btn_rtn) cerr << "Failed to properly init buttons" << endl;

    const bool motor_rtn { MotorController::init() == ReturnCodes::Success };
    rtn &= motor_rtn;
    if (!motor_rtn) cerr << "Failed to properly init motors" << endl;

    const bool servo_rtn { ServoController::init() == ReturnCodes::Success };
    rtn &= servo_rtn;
    if (!servo_rtn) cerr << "Failed to properly init servos" << endl;

    const bool dist_rtn { DistSensor::init() == ReturnCodes::Success };
    rtn &= dist_rtn;
    if (!dist_rtn) cerr << "Failed to properly init ultrasonic distance sensor" << endl;


    // set callback so that when the button is pressed, the LED's state changes
    ButtonController::setBtnCallback([&](const std::string& color, const bool btn_state){
        if(setLED(color, btn_state) != ReturnCodes::Success) {
            cerr << "Failed to set LED " << color << " to " << btn_state << endl;
        }
    });

    return rtn ? ReturnCodes::Success: ReturnCodes::Error;
}

ReturnCodes GPIOController::setShouldThreadExit(const bool new_status) const {
    // only return success if both were successful
    bool rtn {true};
    rtn &= LEDController::setShouldThreadExit(new_status)       == ReturnCodes::Success;
    rtn &= ButtonController::setShouldThreadExit(new_status)    == ReturnCodes::Success;
    rtn &= MotorController::setShouldThreadExit(new_status)     == ReturnCodes::Success;
    rtn &= ServoController::setShouldThreadExit(new_status)     == ReturnCodes::Success;
    rtn &= DistSensor::setShouldThreadExit(new_status)          == ReturnCodes::Success;
    return rtn ? ReturnCodes::Success : ReturnCodes::Error;
}

bool GPIOController::getShouldThreadExit() const {
    return LEDController::getShouldThreadExit() 
        || ButtonController::getShouldThreadExit() 
        || MotorController::getShouldThreadExit()  
        || ServoController::getShouldThreadExit()  
        || DistSensor::getShouldThreadExit()
        ;
}


ReturnCodes GPIOController::gpioHandlePkt(const Network::CommonPkt& pkt) const {
    bool rtn {true}; // changes to false if any return not Success

    // handle leds
    const auto& leds_status { pkt.cntrl.led };
    rtn &= ReturnCodes::Success == setLED("blue",          leds_status.blue);
    rtn &= ReturnCodes::Success == setLED("green",         leds_status.green);
    rtn &= ReturnCodes::Success == setLED("red",           leds_status.red);
    rtn &= ReturnCodes::Success == setLED("yellow",        leds_status.yellow);

    // handle motors
    const auto& motor_status { pkt.cntrl.motor };
    rtn &= ChangeMotorDir(
        motor_status.forward,
        motor_status.backward,
        motor_status.left,
        motor_status.right
    ) == ReturnCodes::Success;

    // handle servos
    const auto& servo_status { pkt.cntrl.servo };
    rtn &= IncrementServoPos(Servo::I2C_ServoAddr::YAW, servo_status.horiz) == ReturnCodes::Success;
    rtn &= IncrementServoPos(Servo::I2C_ServoAddr::PITCH, servo_status.vert) == ReturnCodes::Success;

    return rtn ? ReturnCodes::Success : ReturnCodes::Error;
}


ReturnCodes GPIOController::run(const CLI::Results::ParseResults& flags) {
    // get required variables from flag mapping
    const auto& mode        {flags.at(CLI::Results::ParseKeys::MODE)};
    const auto& colors      {Helpers::splitStr(',', flags.at(CLI::Results::ParseKeys::COLORS))};
    const auto& interval    {
                                static_cast<unsigned int>(
                                    std::stoi(flags.at(CLI::Results::ParseKeys::INTERVAL))
                                )
                            };
    const auto& duration    {std::stoi(flags.at(CLI::Results::ParseKeys::DURATION))};
    const auto& rate        {
                                static_cast<unsigned int>(
                                    std::stoi(flags.at(CLI::Results::ParseKeys::RATE))
                                )
                            };

    // start up selected function based on mode in a thread (joined in destructor)
    run_thread = std::thread{
        std::bind(&GPIOController::callSelFn, this,
            // the actual arguments needed by the function
            mode,
            colors,
            interval,
            duration,
            rate
        )
    };

    return ReturnCodes::Success;
}

void GPIOController::ObstacleAvoidanceTest(
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

    // helps keep track if duration is up (needed bc loop may take awhilem but can be split & stopped in piecemeal)
    auto isDurationUp = [&]()->bool {
        // if duration == -1 : run forever
        return
            duration != -1 &&
            Helpers::Timing::hasTimeElapsed(start_time, std::chrono::milliseconds(duration));
    };
    auto shouldStop = [&]()->bool {
        return GPIOController::getShouldThreadExit() || isDurationUp();
    };

    // contains target distances (in cm)
    constexpr int TargetDist {25};

    // move forward until too close to object, 
    // use servos to sweep and find open path
    // turn in that direction and repeat
    while (!shouldStop()) {
        const auto dist_cm {DistSensor::GetDistanceCm()};
        if (dist_cm.has_value() && *dist_cm > TargetDist) {
            // forward
            MotorController::ChangeMotorDir(Interface::YDirection::FORWARD, Interface::XDirection::NONE);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            continue;
        }
        
        // found obstacle & need to find new direction
        // sweep servos left (0) -> middle (90) -> right (180)
        // (define local enum to make easy)
        enum AnglePos {
            Left=0,
            Middle=90,
            Right=180
        };
        const std::array<int, 3> sweep_angles = {Left, Middle, Right};
        for (const auto& ang : sweep_angles) {
            // move servo & pause to allow it to adjust/reach pos
            ServoController::SetServoPos(Servo::I2C_ServoAddr::YAW, ang);
            std::this_thread::sleep_for(std::chrono::milliseconds(200));

            // check if valid distance and far enough away
            const auto pot_dir_dist_cm {DistSensor::GetDistanceCm()};
            if (!pot_dir_dist_cm.has_value()) continue;

            if(DistSensor::isVerbose())
                cout << "dist = " << *pot_dir_dist_cm << " (" << ang << "°)" << endl;

            if (*pot_dir_dist_cm >= TargetDist) {
                // set servo back to middle & turn until sees clear
                ServoController::SetServoPos(Servo::I2C_ServoAddr::YAW, Middle);
                // pause to allow servo to reach middle before starting to turn
                std::this_thread::sleep_for(std::chrono::milliseconds(300));

                // turn in direction that is free
                if (ang == Left) {
                    if(DistSensor::isVerbose()) cout << "Turning Left" << endl;
                    MotorController::ChangeMotorDir(Interface::YDirection::FORWARD, Interface::XDirection::LEFT);
                } else if (ang == Middle) {
                    if(DistSensor::isVerbose()) cout << "Turning Middle" << endl;
                    MotorController::ChangeMotorDir(Interface::YDirection::FORWARD, Interface::XDirection::NONE);
                } else if (ang == Right) {
                    if(DistSensor::isVerbose()) cout << "Turning Right" << endl;
                    MotorController::ChangeMotorDir(Interface::YDirection::FORWARD, Interface::XDirection::RIGHT);
                }

                auto turn_dist_cm {DistSensor::GetDistanceCm()};
                while(!shouldStop() && turn_dist_cm.has_value() && *turn_dist_cm < TargetDist) {
                    std::this_thread::sleep_for(std::chrono::milliseconds(100));
                    turn_dist_cm = DistSensor::GetDistanceCm();
                    if(DistSensor::isVerbose()) cout << "turning w/ dist = " << *turn_dist_cm << "cm" << endl;
                }

                break; // break out of servo sweep for-loop
            }
        } // end of servo sweep for loop
    }
}

/********************************************* Helper Functions ********************************************/

void GPIOController::doNothing() const {
    // sometimes you just gotta be a bit sassy
    // cout << "You chose the option to do nothing... you should rethink your life choices" << endl;
}

void GPIOController::callSelFn(
    const std::string& mode,
    const std::vector<std::string>& colors,
    const unsigned int& interval,
    const int& duration,
    const unsigned int& rate
) const {
    mode_to_action.searchAndCall<void>(
        *this, // need to pass reference to this object
        mode, // key to the function to call
        // pass the actual params needed by functions
        colors,
        interval,
        duration,
        rate
    );
}


ModeMap GPIOController::createFnMap() {
    ModeMap to_rtn;
    // TODO: Figure out way to make reinterpret_cast automatic
    to_rtn["blink"]       = reinterpret_cast<void(LEDController::*)()>(&LEDController::blinkLEDs);
    to_rtn["intensity"]   = reinterpret_cast<void(LEDController::*)()>(&LEDController::LEDIntensity);
    to_rtn["btns"]        = reinterpret_cast<void(ButtonController::*)()>(&ButtonController::detectBtnPress);
    to_rtn["motors"]      = reinterpret_cast<void(MotorController::*)()>(&MotorController::testMotorsLoop);
    to_rtn["servos"]      = reinterpret_cast<void(GPIOController::*)()>(&ServoController::testServos);
    to_rtn["ultrasonic"]  = reinterpret_cast<void(GPIOController::*)()>(&DistSensor::testDistSensor);
    to_rtn["obstacle"]    = reinterpret_cast<void(GPIOController::*)()>(&GPIOController::ObstacleAvoidanceTest);
    to_rtn["server"]      = reinterpret_cast<void(GPIOController::*)()>(&GPIOController::doNothing);
    to_rtn["client"]      = reinterpret_cast<void(GPIOController::*)()>(&GPIOController::doNothing);
    to_rtn["camera"]      = reinterpret_cast<void(GPIOController::*)()>(&GPIOController::doNothing);
    to_rtn["none"]        = reinterpret_cast<void(GPIOController::*)()>(&GPIOController::doNothing);
    return to_rtn;
}


}; // end of gpio namespace

}; // end of RPI namespace
