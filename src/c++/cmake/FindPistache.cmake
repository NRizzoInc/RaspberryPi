# FindPistache.cmake - Try to find the Pistache c++ library for web apps
# Once done this will define
#
# Pistache_LIBRARIES    - Location of Pistache bins
# Pistache_INCLUDE_DIR  - Location of the Pistache header

# paths come from Pistache ./build script (called from setup script)
set(Pistache_BINS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern/pistache/build/src")
set(Pistache_HEADERS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/extern/pistache/include/")
find_library(Pistache_LIBRARIES
    NAMES pistache libpistache libpistache.a
    HINTS ${Pistache_BINS_DIR}
)
find_path(Pistache_INCLUDE_DIR
    NAMES endpoint.h pistache/endpoint.h 
    HINTS ${Pistache_HEADERS_DIR}
)

# check if not found (need to call build script)
if (NOT Pistache_LIBRARIES)
    message(WARNING "Pistache library not found -- calling build script")
    execute_process(
        # call from helper directly to avoid using sudo
        COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/install/helpers/pistache.sh --mode install
    )
    # try to find again now that it was installed
    find_library(Pistache_LIBRARIES
        NAMES pistache libpistache libpistache.a
        HINTS ${Pistache_BINS_DIR}
    )
endif()


MARK_AS_ADVANCED(Pistache_LIBRARIES Pistache_INCLUDE_DIR)
