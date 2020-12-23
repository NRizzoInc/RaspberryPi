# FindJSON.cmake - Try to find the JSON library
# Once done this will define
#
# Note: Using https://github.com/nlohmann/json
# JSON_FOUND       - Found the JSON library path
# JSON_INCLUDE_DIR - Location of the JSON header

# set statically, because these are pinned by git and not version controlled
SET(JSON_FOUND True)
SET(JSON_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/extern/JSON/) # give relative path to include dir
