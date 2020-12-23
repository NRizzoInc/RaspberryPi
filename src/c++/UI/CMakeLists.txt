# use this to find in main dir
add_library(RPI_UI
    backend.cpp
    web_handlers.cpp
) 

target_link_libraries(RPI_UI
    ${Pistache_LIBRARIES}
    stdc++fs # -lstdc++fs == use <filesystem>
)

target_compile_options(RPI_UI
    PRIVATE
)
