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
    // create gpio obj to controll rpi
    gpio::GPIO_Controller gpio_obj;

    // setup ctrl+c handler w/ callback to stop threads
    create_signal_handler_callback(std::function<void()>([&]() {
            gpio_obj.setShouldThreadExit(true);
        })
    );

    cout << "Blinking red led" << endl;
    gpio_obj.blinkLEDs({"red", "green"});

    return 0;
}