#include "Ultrasonic.h"


// to keep lines short
using std::cout;
using std::cerr;
using std::endl;


namespace RPI {
namespace gpio {
namespace Ultrasonic {

/********************************************** Constructors **********************************************/

DistSensor::DistSensor(const bool verbosity)
    : GPIOBase{verbosity}
{
    // stub
}

DistSensor::~DistSensor() {
    // set all ultrasonic sensor's pins to off at end
    if (getIsInit()) {

        setIsInit(false);
    }
}

ReturnCodes DistSensor::init() const {
    // if already init, stop now
    if (getIsInit()) return ReturnCodes::Success;

    if(GPIOBase::init() != ReturnCodes::Success) {
        return ReturnCodes::Error;
    }

    // setup ultrasonic pins
    pinMode(Ultrasonic::PinType::ECHO, INPUT);
    pinMode(Ultrasonic::PinType::TRIGGER, OUTPUT);
    pullUpDnControl(Ultrasonic::PinType::ECHO, PUD_UP);

    setIsInit(true);
    return ReturnCodes::Success;
}

/********************************************* Getters/Setters *********************************************/


/****************************************** Ultrasonic Functions *******************************************/

std::optional<float> DistSensor::GetDistance() const {
    // perform distance check 5 times and get median value
    std::vector<float> distances_cm;
    for (int i = 0; i < 5; i++) {
        auto pulse_duration {WaitForEcho()};
        // if invalid reading, discard the entry
        if (!pulse_duration.has_value()) {
            continue; // dont try to set value
        }

        // math is being done in microseconds
        const auto pulse_dur_us {std::chrono::duration_cast<std::chrono::microseconds>(*pulse_duration)};
        const float dist_cm { static_cast<float>(pulse_dur_us.count() / 58) };
        distances_cm.push_back(dist_cm);
    }

    // compute median
    const std::size_t size {distances_cm.size()};
    if (size == 0) {
        // if no elements, none of the readings were successful so stop
        return std::nullopt;
    }
    std::sort(distances_cm.begin(), distances_cm.end());

    // odd (easy, just middle element)
    // even (compute avg between 2 middle elements)
    return size % 2 != 0 ?
        distances_cm[size / 2] : static_cast<float>((distances_cm[(size / 2) - 1] + distances_cm[size/2]) / 2.0);

}

void DistSensor::testDistSensor(
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

    while (!DistSensor::getShouldThreadExit() && !isDurationUp()) {
        // bc of threading, have to get distance before printing or else stream will disjoin print strings
        const auto dist {GetDistance()};
        if (dist.has_value()) {
            cout << "Distance: " << *dist << "cm" << endl;
        } else {
            cerr << "Error: Failed to get distance" << endl;
        }
    }
}

/********************************************* Helper Functions ********************************************/


void DistSensor::SendTriggerPulse() const {
    // send pulse in order (pause in between to allow update)
    digitalWrite(Ultrasonic::PinType::TRIGGER, Ultrasonic::DistPulseOrder::First);
    std::this_thread::sleep_for(std::chrono::microseconds(150));
    digitalWrite(Ultrasonic::PinType::TRIGGER, Ultrasonic::DistPulseOrder::Second);
}

ReturnCodes DistSensor::WaitForEdge(
    const bool edge_val,
    const std::chrono::steady_clock::duration timeout
) const {
    auto start_time {std::chrono::steady_clock::now()};
    while (digitalRead(Ultrasonic::PinType::ECHO) != edge_val) {
        if (Helpers::Timing::hasTimeElapsed(start_time, timeout)) {
            return ReturnCodes::Timeout;
        }
    }

    return ReturnCodes::Success;
}


std::optional<std::chrono::steady_clock::duration> DistSensor::WaitForEcho(
    const std::chrono::steady_clock::duration timeout
) const {
    // assume SendTriggerPulse() was called and map time between First & Second signal
    // (if not, it'll just timeout)

    if (WaitForEdge(Ultrasonic::DistPulseOrder::First, timeout) == ReturnCodes::Timeout) {
        // stop if timeout
        if(isVerbose()) cerr << "Error: Ultrasonic sensor timeout (start)" << endl;
        return std::nullopt;
    }

    // pulse has acutally been sent out at this point and should start tracking time diff
    auto pulse_start {std::chrono::steady_clock::now()};

    if (WaitForEdge(Ultrasonic::DistPulseOrder::Second, timeout) == ReturnCodes::Timeout) {
        // stop if timeout
        if(isVerbose()) cerr << "Error: Ultrasonic sensor timeout (end)" << endl;
        return std::nullopt;
    }

    auto pulse_stop {std::chrono::steady_clock::now()};

    // return time difference (length of time it took ultrasonic signal to travel two-ways)
    return pulse_stop - pulse_start; 
}


}; // end of Ultrasonic namespace
}; // end of gpio namespace
}; // end of RPI namespace
