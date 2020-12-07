# FindCXXOpts.cmake - Try to find the CLI library
# Once done this will define
#
# CXXOPTS_FOUND       - Found the cxxopts path
# CXXOPTS_INCLUDE_DIR - Location of the cxxopts header

# set statically, because these are pinned by git and not version controlled
SET(CLI11_FOUND True)
SET(CLI11_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/CLI11/include/) # give relative path to include dir
