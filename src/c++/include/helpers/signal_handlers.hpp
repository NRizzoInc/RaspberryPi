#ifndef SIGNAL_HANDLERS_HPP
#define SIGNAL_HANDLERS_HPP

// Standard Includes
#include <iostream>
#include <csignal>
#include <functional>

// private
namespace {
    void signal_handler(int signal) {
        std::cout << "Caught ctrl+c: " << signal << std::endl;
        std::exit(EXIT_SUCCESS);
    }
} // namespace

void  create_signal_handler() {
    std::signal(SIGINT, signal_handler);
}



#endif