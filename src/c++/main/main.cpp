// Standard Includes
#include <iostream>
#include <functional>
#include <csignal>

// Our Includes
#include <GPIO_Controller.h>

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

    cout << "Changing led intensity" << endl;
    gpio_obj.LEDIntensity({"red", "green"});

    return 0;
}