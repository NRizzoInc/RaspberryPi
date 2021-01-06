# use this to find in main dir
add_library(RPI_Network
    tcp_base.cpp
    tcp_server.cpp
    tcp_client.cpp
    packet.cpp
) 

target_link_libraries(RPI_Network
)

target_compile_options(RPI_Network
    PRIVATE
)

