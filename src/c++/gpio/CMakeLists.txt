############################# Library for utilizing GPIO Code in Main #############################
add_library(GPIO_Controller
    GPIO_Controller.cpp
)

target_link_libraries(GPIO_Controller
    LedBtn_Controller
    MotorServo_Controller
)

target_compile_options(GPIO_Controller
    PRIVATE
)

############################ sub libraries required for GPIO_Controller ###########################


#################### sub libraries for motors and servos ####################
add_library(MotorServo_Controller
    GPIO_Base.cpp
    Motor_Controller.cpp
    Servo_Controller.cpp
)

target_link_libraries(MotorServo_Controller
    ${WiringPi_LIBRARIES}
)

target_compile_options(MotorServo_Controller
    PRIVATE
)

#################### sub libraries for leds & buttons ####################
add_library(LedBtn_Controller
    GPIO_Base.cpp
    LED_Controller.cpp
    Button_Controller.cpp
)

target_link_libraries(LedBtn_Controller
    ${WiringPi_LIBRARIES}
)

target_compile_options(LedBtn_Controller
    PRIVATE
)
