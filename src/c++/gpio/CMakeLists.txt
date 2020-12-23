# use this to find in main dir
add_library(GPIO_Controller
    GPIO_Controller.cpp
    LED_Controller.cpp
    Button_Controller.cpp
    GPIO_Base.cpp
) 

target_link_libraries(GPIO_Controller
    ${WiringPi_LIBRARIES}
)

target_compile_options(GPIO_Controller
    PRIVATE
)
