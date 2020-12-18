# FindCrow.cmake - Try to find the CLI library
# Once done this will define
#
# CROW_INCLUDE_DIR  - Location of the crow header
# CROW_LIBS         - Location of crow static libs (built locally)

# set statically, because these are pinned by git & by external projects build system
find_path(CROW_INCLUDE_DIR
    NAMES crow.h crow/crow.h
    HINTS ${CMAKE_CURRENT_SOURCE_DIR}/extern/crow/include/
)

# check if not found (need to call build script)
if (NOT CROW_INCLUDE_DIR)
    message(WARNING "Looking for Crow Library: ${CROW_LIBS}")
    message(WARNING "Crow library not found -- calling build script")
    execute_process(
        # call from helper directly to avoid using sudo
        COMMAND ${CMAKE_SOURCE_DIR}/install/helpers/Crow.sh --mode install
    )
endif()

# include other necessary libs
set(CROW_LIBS ${CROW_LIBS})

MARK_AS_ADVANCED(CROW_INCLUDE_DIR)
