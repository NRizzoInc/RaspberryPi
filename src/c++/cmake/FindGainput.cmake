# FindGainput.cmake - Try to find the CLI library
# Once done this will define
#
# Gainput_INCLUDE_DIR  - Location of the gainput header
# Gainput_LIBS         - Location of gainput static libs (built locally)

# set statically, because these are pinned by git & by external projects build system
find_library(Gainput_LIBS
    NAMES gainput libgainput
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/gainput/build/lib/
)
find_path(Gainput_INCLUDE_DIR
    NAMES gainput.h gainput/gainput.h
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/gainput/lib/include/
)

# check if not found (need to call build script)
if (NOT Gainput_LIBS)
    message(WARNING "gainput library not found -- calling build script")
    execute_process(
        # call from helper directly to avoid using sudo
        COMMAND ${CMAKE_SOURCE_DIR}/install/helpers/Gainput.sh --mode install
    )
    # try to find again now that it was installed
    find_library(Gainput_LIBS NAMES gainput)
    find_path(Gainput_INCLUDE_DIR NAMES gainput.h)
endif()

# include other necessary libs
set(Gainput_LIBS ${Gainput_LIBS} X11)

MARK_AS_ADVANCED(Gainput_INCLUDE_DIR Gainput_LIBS)
