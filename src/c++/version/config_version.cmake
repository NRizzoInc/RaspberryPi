# this script should be called with `add_custom_target` in a local CMakeLists.txt files when building a "library" containing the version info
# this library will be rebuilt each time `make` is called to make sure it is up to date
# why necessary: https://stackoverflow.com/a/35302833/13933174

# include method for getting current git versioning info
include(GetGitRevisionDescription REQUIRED) # https://stackoverflow.com/a/4318642/13933174
git_describe_working_tree(GIT_DESC) # define GIT_DESC

#################### some additional git defines not included in package

# git commit hash
execute_process(COMMAND
    "${GIT_EXECUTABLE}" describe --match=NeVeRmAtCh --always --abbrev=40 --dirty
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_SHA1
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)

# the commit's branch
execute_process(COMMAND
    "${GIT_EXECUTABLE}" rev-parse --abbrev-ref HEAD
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_BRANCH
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)

# the date of the commit
execute_process(COMMAND
    "${GIT_EXECUTABLE}" log -1 --format=%ad --date=local
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_DATE
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)

# the subject of the commit
execute_process(COMMAND
    "${GIT_EXECUTABLE}" log -1 --format=%s
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE GIT_COMMIT_SUBJECT
    ERROR_QUIET OUTPUT_STRIP_TRAILING_WHITESPACE
)

# copy file
configure_file(${VERSION_SRC_PATH} ${VERSION_DST_PATH})
