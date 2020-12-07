# main cmake for checkers project
cmake_minimum_required(VERSION 2.8)

add_executable(gpio_controller
    GPIOController.cpp
)

# include created libs from other dirs -- comes after add executable
target_link_libraries(gpio_controller
)