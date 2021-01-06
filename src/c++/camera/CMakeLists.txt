# use this to find in main dir
add_library(RPI_Camera
    rpi_camera.cpp
) 

target_link_libraries(RPI_Camera
    ${Raspicam_LIBRARIES}
)

target_compile_options(RPI_Camera
    PRIVATE
)

