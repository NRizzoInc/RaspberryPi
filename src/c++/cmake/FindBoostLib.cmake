# Find Boost libraries -- https://cmake.org/cmake/help/v3.14/module/FindBoost.html
# Once done this will define
#
# Boost_LIBRARIES    - Location of the Boost compile libraries
# Boost_INCLUDE_DIRS - Location of the Boost include header files


# set(Boost_ADDITIONAL_VERSIONS "1.48" "1.48.0")
set(Boost_USE_STATIC_LIBS        ON)  # only find static libs
set(Boost_USE_DEBUG_LIBS         OFF) # ignore debug libs and
set(Boost_USE_RELEASE_LIBS       ON)  # only find release libs
set(Boost_USE_MULTITHREADED      ON)
set(Boost_USE_STATIC_RUNTIME    OFF)
find_package(Boost COMPONENTS system thread regex filesystem REQUIRED)
MARK_AS_ADVANCED(Boost_INCLUDE_DIRS Boost_LIBRARIES)
