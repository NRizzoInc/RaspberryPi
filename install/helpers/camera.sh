#!/bin/bash
#@file: Builds and Installs External OpenCV Library

# CLI Flags
print_flags () {
    echo "========================================================================================================================="
    echo "Usage: camera.sh"
    echo "========================================================================================================================="
    echo "Helper utility to setup the OpenCV external library in this repo to stream video to client"
    echo "========================================================================================================================="
    echo "How to use:" 
    echo "  To Start: ./camera.sh [flags]"
    echo "========================================================================================================================="
    echo "Available Flags:"
    echo "    -i | --extern-dir: Path to the extern dir (defaults to git pinned path)"
    echo "    -m | --mode: [install, clean]"
    echo "    -h | --help: This message"
    echo "========================================================================================================================="
}

# parse command line args
mode="install" # default to install
externDir="$(readlink -fm "$0"/../../../extern/)" # default to git pinned path
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -i | --extern-dir )
            externDir="$2"
            shift
            ;;
        -m | --mode )
            mode="$2"
            if [[ ${mode} != "install" && ${mode} != "clean" ]]; then
                echo "Mode can be [install, clean]"
                exit 1
            fi
            shift
            ;;
        -h | --help )
            print_flags
            exit 0
            ;;
        * )
            echo "... Unrecognized Command: $1"
            print_flags
            exit 1
    esac
    shift
done

# deduse library paths
opencvRootDir="${externDir}/OpenCV"
opencvContribDir="${opencvRootDir}/opencv_contrib"
opencvDir="${opencvRootDir}/opencv"
if [[ ${mode} == "install" ]]; then

    # install gpio lib, have to be in the correct dir for this (chain commands)
    echo "=============== Bulding and Installing OpenCV Library! ==============="
    # following these instructions: https://linuxize.com/post/how-to-install-opencv-on-ubuntu-18-04/
    cd "${opencvDir}" && \
        mkdir -p build && \
        cd build && \
        cmake \
            -D CMAKE_BUILD_TYPE=RELEASE \
            -D CMAKE_INSTALL_PREFIX=/usr/local \
            -D INSTALL_C_EXAMPLES=ON \
            -D INSTALL_PYTHON_EXAMPLES=ON \
            -D OPENCV_GENERATE_PKGCONFIG=ON \
            -D OPENCV_EXTRA_MODULES_PATH="${opencvContribDir}/modules" \
            -D BUILD_EXAMPLES=ON .. && \
        make -j"$(nproc)" && \
        make install || \
        echo "Failed to build: try running ./install/helpers/linux_pkgs.sh to get requirements"

else # clean
    echo "====================== Cleaning OpenCV Library! ======================"
    cd "${opencvDir}" && \
        rm -rf build || \
        echo "Failed to clean OpenCV"
fi