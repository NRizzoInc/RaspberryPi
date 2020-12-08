#ifndef SIGNAL_HANDLERS_HPP
#define SIGNAL_HANDLERS_HPP

// Standard Includes
#include <iostream>
#include <csignal>
#include <functional>

// Our Includes
#include "constants.h"

// private
namespace {
    template <typename ... FnType, typename ... FnArgs>
    /**
     * @brief Generates a standard signal handler, but with a callback
     * @param callback The additional function to call
     * @param args The callback's arguments (optional -- can omit)
     * @return std::function<void(int)> The standard type of function needed for signal
     */
    std::function<void(int)> signal_handler_generator(std::function<void(FnType...)> const & callback, FnArgs && ... args) {
        return std::function<void(int)>([&](int signal) {
            std::cout << "Caught ctrl+c " << signal << std::endl;

            // call the passed function and forward all necessary arguments
            if (callback) {

                // find out size of additional arguments (or call w/o if none)
                const int arg_list_size = sizeof...(args);
                if (arg_list_size != 0) {
                    // use "..." to unpack args
                    callback(std::forward<FnArgs>(args)...);
                } else {
                    callback();
                }
            }
        });
    }
} // namespace

/**
 * @brief Create a signal handler object with a callback
 * @param callback function that has void return
 * @param args callback function arguments (optional -- can omit)
 */
template <typename ... FnType, typename ... FnArgs>
ReturnCodes create_signal_handler_callback(std::function<void(FnType...)> const & callback, FnArgs && ... args) {
    // have to convert std::function => c-style void* function ptr
    std::function<void(int)> cpp_hander = signal_handler_generator(callback, args...);
    sighandler_t* handler_fn_ptr = cpp_hander.target<sighandler_t>(); 

    // actually call signal function with c-style function pointer
    std::signal(SIGINT, *handler_fn_ptr);
    return ReturnCodes::Success;
}



#endif