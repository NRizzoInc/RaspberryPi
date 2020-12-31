# FindRaspicam.cmake - Try to find the Raspicam c++ library for handling the camera module
# Once done this will define
#
# Raspicam_LIBRARIES    - Location of Raspicam bins
# Raspicam_INCLUDE_DIR  - Location of the Raspicam header

# paths come from Raspicam build script (called from setup script)
set(Raspicam_BINS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern/camera/raspicam/build/src")
set(Raspicam_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern/camera/raspicam/src")
find_library(Raspicam_LIBRARIES
    NAMES libraspicam.so raspicam raspicam_cv libraspicam_cv.so libraspicam_cv
    HINTS ${Raspicam_BINS_DIR}
)
find_path(Raspicam_INCLUDE_DIR
    NAMES raspicam/raspicam.h raspicam.h
    HINTS ${Raspicam_HEADERS_DIR}
)

# check if not found (need to call build script)
if (NOT Raspicam_LIBRARIES)
    message(WARNING "Raspicam library not found -- calling build script")
    execute_process(
        # call from helper directly to avoid using sudo
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/install/helpers/camera.sh --mode install
    )
    # try to find again now that it was installed
    find_library(Raspicam_LIBRARIES
    NAMES libraspicam.so raspicam raspicam_cv libraspicam_cv.so libraspicam_cv
    HINTS ${Raspicam_BINS_DIR}
)
endif()


MARK_AS_ADVANCED(Raspicam_LIBRARIES Raspicam_INCLUDE_DIR)
