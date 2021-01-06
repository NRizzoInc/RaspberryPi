# FindRaspicam.cmake - Try to find the Raspicam c++ library for handling the camera module
# Once done this will define
#
# Raspicam_LIBRARIES    - Location of Raspicam bins
# Raspicam_INCLUDE_DIR  - Location of the Raspicam header

# paths come from Raspicam build script (called from setup script)
set(Raspicam_ROOT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern/camera/raspicam/")
set(Raspicam_BUILD_DIR "${Raspicam_ROOT_DIR}/build")
set(Raspicam_BINS_DIR "${Raspicam_BUILD_DIR}/src")
set(Raspicam_HEADERS_DIR "${Raspicam_ROOT_DIR}/src")
find_library(Raspicam_LIBRARIES
    NAMES raspicam_cv  
    NAMES raspicam
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
        NAMES raspicam_cv  
        NAMES raspicam
        HINTS ${Raspicam_BINS_DIR}
    )
endif()

# add additional libs/includes needed
find_package(OpenCV REQUIRED) # note: FindOpenCV.cmake provided by apt pkg
if(OpenCV_FOUND)
    set(Raspicam_INCLUDE_DIR
        ${Raspicam_INCLUDE_DIR}
        ${OpenCV_INCLUDE_DIRS}
    )

    set(Raspicam_LIBRARIES 
        ${Raspicam_LIBRARIES}
        ${OpenCV_LIBRARIES}
    )
endif()


MARK_AS_ADVANCED(Raspicam_LIBRARIES Raspicam_INCLUDE_DIR)
