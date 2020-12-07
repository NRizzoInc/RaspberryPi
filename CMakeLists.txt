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
set(CMAKE_CXX_FLAGS "-Wall -Wextra")
set(CMAKE_CXX_FLAGS_DEBUG "-g")
set(CMAKE_CXX_FLAGS_RELEASE "-O3")


# =================================== load Dependencies globally for all subdirs ===================================
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/src/c++/cmake")

# Find the packages
# find_package(BoostLib REQUIRED) # uses FindBoost.cmake in cmake dir
find_package(CLI11 REQUIRED) # using https://cliutils.gitlab.io/CLI11Tutorial/


# Include the package's header files
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/c++/include") # contains OUR headers
include_directories("${CMAKE_CURRENT_SOURCE_DIR}/src/c++/include/helpers")
include_directories(${CLI11_INCLUDE_DIR})



# ===================================== Accept Build Mode for Debug/Release ========================================
# TODO

# ============================================ Build individual subdirs ============================================
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin) # store produce binaries in src/bin/*
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lib)

# add more folders here as needed
# these trigger the CMakeLists.txt file to be called in the listed dirs
add_subdirectory(src/c++/main)
