# FindWiringPi.cmake - Try to find the WiringPi c++ library for RPI GPIOs
# Once done this will define
#
# WiringPi_LIBS         - Location of WiringPi bins
# WiringPi_INCLUDE_DIRS - Location of the WiringPi header

# paths come from WiringPi ./build script (called from setup script)
find_library(WiringPi_LIBS wiringPi)
find_path(WiringPi_INCLUDE_DIRS wiringPi.h)

# check if not found (need to call build script)
if (NOT WiringPi_INCLUDE_DIRS)
    message(WARNING "WiringPi library not found -- calling build script")
    execute_process(
        # call from helper directly to avoid using sudo
        COMMAND ${CMAKE_SOURCE_DIR}/install/helpers/WiringPi.sh --mode install
    )
    # try to find again now that it was installed
    find_library(WiringPi_LIBS NAMES wiringPi)
    find_path(WiringPi_INCLUDE_DIRS NAMES wiringPi.h)
endif()



MARK_AS_ADVANCED(WiringPi_LIBS WiringPi_INCLUDE_DIRS)