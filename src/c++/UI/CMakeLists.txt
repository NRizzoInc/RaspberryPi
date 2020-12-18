# use this to find in main dir
add_library(RPI_UI
    EventListener.cpp
) 

target_link_libraries(RPI_UI
    ${Boost_LIBRARIES}
)

target_compile_options(RPI_UI
    PRIVATE
)
