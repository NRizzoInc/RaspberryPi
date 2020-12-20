# use this to find in main dir
add_library(RPI_UI
    backend.cpp
) 

target_link_libraries(RPI_UI
    ${Pistache_LIBRARIES}
)

target_compile_options(RPI_UI
    PRIVATE
)
