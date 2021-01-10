############################# Library for utilizing Network Code in Main #############################
add_library(RPI_Network
    tcp_server.cpp
    tcp_client.cpp
) 

target_link_libraries(RPI_Network
    RPI_Net_LowLevel
)

target_compile_options(RPI_Network
    PRIVATE
)

############################ sub libraries required for RPI_Network ###########################

add_library(RPI_Net_LowLevel
    tcp_base.cpp
    packet.cpp
)

target_link_libraries(RPI_Net_LowLevel
)

target_compile_options(RPI_Net_LowLevel
    PRIVATE
)
