# FindWiringPi.cmake - Try to find the WiringPi c++ library for RPI GPIOs
# Once done this will define
#
# WiringPi_LIBRARIES    - Location of WiringPi bins
# WiringPi_INCLUDE_DIR  - Location of the WiringPi header

# paths come from WiringPi ./build script (called from setup script)
find_library(WiringPi_LIBRARIES wiringPi)
find_path(WiringPi_INCLUDE_DIR
    NAMES wiringPi.h 
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/WiringPi/wiringPi/
)

# check if not found (need to call build script)
if (NOT WiringPi_LIBRARIES)
    message(WARNING "WiringPi library not found -- calling build script")
    execute_process(
        # call from helper directly to avoid using sudo
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/install/helpers/WiringPi.sh --mode install
    )
    # try to find again now that it was installed
    find_library(WiringPi_LIBRARIES NAMES wiringPi)
    find_path(WiringPi_INCLUDE_DIR NAMES wiringPi.h)
endif()


# include other necessary libs
find_package(Threads REQUIRED) # stock package needed for WiringPi -- link ${CMAKE_THREAD_LIBS_INIT}
set(WiringPi_LIBRARIES 
    ${WiringPi_LIBRARIES}
    ${CMAKE_THREAD_LIBS_INIT}
    crypt   # for undefined reference to `crypt'
    rt      # for undefined reference to `shm_open'
)

MARK_AS_ADVANCED(WiringPi_LIBRARIES WiringPi_INCLUDE_DIR)
