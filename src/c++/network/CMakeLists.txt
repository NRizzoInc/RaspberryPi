# use this to find in main dir
add_library(RPI_Network
    net_common.cpp
    server.cpp
    client.cpp
    packet.cpp
) 

target_link_libraries(RPI_Network
)
