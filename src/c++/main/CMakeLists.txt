cmake_minimum_required(VERSION 2.8)

add_executable(rpi_driver
    main.cpp
)

# include created libs from other dirs -- comes after add executable
target_link_libraries(rpi_driver
    RPI_Version
    GPIO_Controller
    GPIO_CLI_Parser
    RPI_Network
    RPI_UI
    RPI_Camera # evenutally include as part of client in RPI_Network
)

target_compile_options(rpi_driver
    PRIVATE
)
