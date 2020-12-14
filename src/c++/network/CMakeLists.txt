# use this to find in main dir
add_library(RPI_Network
    tcp_base.cpp
    server.cpp
    client.cpp
    packet.cpp
) 

target_link_libraries(RPI_Network
)

target_compile_options(RPI_Network
    PRIVATE
)

