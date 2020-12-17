# use this to find in main dir
add_library(RPI_UI
    EventListener.cpp
) 

target_link_libraries(RPI_UI
    ${Gainput_LIBS}
)

target_compile_options(RPI_UI
    PRIVATE
)
