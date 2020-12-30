# use this to find in main dir
add_library(RPI_Camera
    rpi_cam.cpp
) 

target_link_libraries(RPI_Camera
)

target_compile_options(RPI_Camera
    PRIVATE
)

