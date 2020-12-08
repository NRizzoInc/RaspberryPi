# FindWiringPi.cmake - Try to find the WiringPi c++ library for RPI GPIOs
# Once done this will define
#
# WiringPi_LIBS         - Location of WiringPi bins
# WiringPi_INCLUDE_DIRS - Location of the WiringPi header

# check if not found (need to call build script)
if (NOT EXISTS wiringPi.h OR NOT EXISTS wiringPi)
    message(WARNING "WiringPi library not found -- calling build script")
    execute_process(
        # call from helper directly to avoid using sudo
        COMMAND ${CMAKE_SOURCE_DIR}/install/helpers/WiringPi.sh --mode install
    )
endif()

# paths come from WiringPi ./build script (called from setup script)
find_library(WiringPi_LIBS NAMES wiringPi)
find_path(WiringPi_INCLUDE_DIRS NAMES wiringPi.h)

MARK_AS_ADVANCED(WiringPi_LIBS WiringPi_INCLUDE_DIRS)