// Standard Includes
#include <iostream>

// Our Includes
#include <GPIO_Controller.h>
#include "signal_handlers.hpp"

using std::cout;
using std::cerr;
using std::endl;

int main() {
    // setup ctrl+c handler
    create_signal_handler();

    gpio::GPIO_Controller gpio_obj;

    cout << "Blinking red led" << endl;
    gpio_obj.blinkLEDs({"red", "green"});

    return 0;
}