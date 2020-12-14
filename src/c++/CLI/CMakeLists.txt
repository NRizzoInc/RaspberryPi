# use this to find in main dir
add_library(GPIO_CLI_Parser
    CLI_Parser.cpp
) 

target_link_libraries(GPIO_CLI_Parser
)

target_compile_options(GPIO_CLI_Parser
    PRIVATE
)