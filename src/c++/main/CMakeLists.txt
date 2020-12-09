cmake_minimum_required(VERSION 2.8)

add_executable(rpi_driver
    main.cpp
    CLI_Parser.cpp
)

# include created libs from other dirs -- comes after add executable
target_link_libraries(rpi_driver
    GPIO_Controller
)