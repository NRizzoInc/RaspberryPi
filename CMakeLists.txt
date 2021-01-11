cmake_minimum_required(VERSION 2.8)
project (RPI)

# use c++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# use gcc/++7
set(CMAKE_C_COMPILER /usr/bin/gcc-7 CACHE PATH "" FORCE)
set(CMAKE_CXX_COMPILER /usr/bin/g++-7 CACHE PATH "" FORCE)

# set build type based on makefile target
# CMAKE_BUILD_TYPE set in Makefile
# Ignore Pi Specific Warning from JSON lib: -Wno-psabi
set(CMAKE_CXX_FLAGS "-Wall -Wextra -Wno-psabi")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")

# =================================== load Dependencies globally for all subdirs ===================================
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/src/c++/cmake")

# Find the packages
# find_package(BoostLib REQUIRED) # (really just finds Boost_* but causes an infinite loop since script searches for Boost)
find_package(Git REQUIRED) # predfined package included in system
find_package(JSON REQUIRED) # using https://github.com/nlohmann/json
find_package(CLI11 REQUIRED) # using https://cliutils.gitlab.io/CLI11Tutorial/
find_package(WiringPi REQUIRED) # using https://github.com/WiringPi/WiringPi
find_package(Pistache REQUIRED) # using https://github.com/pistacheio/pistache
find_package(Raspicam REQUIRED) # using https://github.com/cedricve/raspicam

# Include the package's header files
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/c++/include") # contains OUR headers
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/c++/include/helpers")
include_directories(SYSTEM ${JSON_INCLUDE_DIR}) # use system to ingore release warnings from this library
include_directories(${CLI11_INCLUDE_DIR})
include_directories(${WiringPi_INCLUDE_DIR}) # pair with target_link_libraries(<projName> ${WiringPi_LIBS})
include_directories(${Pistache_INCLUDE_DIR}) # pair with Pistache_LIBRARIES
include_directories(${Raspicam_INCLUDE_DIR}) # pair with Raspicam_LIBRARIES
# include_directories(${Boost_INCLUDE_DIRS}) # pair with Boost_LIBRARIES


# ============================================ Build individual subdirs ============================================
if(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)
    set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin) # store produce binaries in RaspberryPi/bin/*
endif(NOT DEFINED CMAKE_RUNTIME_OUTPUT_DIRECTORY)

if(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)
    set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)
endif(NOT DEFINED CMAKE_LIBRARY_OUTPUT_DIRECTORY)


# add more folders here as needed (order matters, executable last with most independent first)
# these trigger the CMakeLists.txt file to be called in the listed dirs
add_subdirectory(src/c++/version) # Get Git Commit/Revision Info: https://stackoverflow.com/a/21028226/13933174
add_subdirectory(src/c++/camera)
add_subdirectory(src/c++/network)
add_subdirectory(src/c++/gpio)
add_subdirectory(src/c++/CLI)
add_subdirectory(src/c++/UI)
add_subdirectory(src/c++/main) # executable
