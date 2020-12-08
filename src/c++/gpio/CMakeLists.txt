# use this to find in main dir
add_library(GPIO_Controller
    GPIO_Controller.cpp
) 

target_link_libraries(GPIO_Controller
    ${WiringPi_LIBS}
)