// Standard Includes
#include <iostream>
#include <functional>

// Our Includes
#include <GPIO_Controller.h>
#include "signal_handlers.hpp"

using std::cout;
using std::cerr;
using std::endl;

int main() {
    // create single static gpio obj to controll rpi
    // static needed so it can be accessed in lambda
    static gpio::GPIO_Controller gpio_obj;

    // setup ctrl+c handler w/ callback to stop threads
    std::signal(SIGINT, [](int signum) {
        cout << "Caught ctrl+c: " << signum << endl;
        gpio_obj.setShouldThreadExit(true);
    });

    cout << "Blinking red led" << endl;
    gpio_obj.blinkLEDs({"red", "green"});

    return 0;
}