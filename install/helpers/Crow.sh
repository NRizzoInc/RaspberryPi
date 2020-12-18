#!/bin/bash
#@file: Builds and Installs External Crow Library

# CLI Flags
print_flags () {
    echo "========================================================================================================================="
    echo "Usage: Crow.sh"
    echo "========================================================================================================================="
    echo "Helper utility to setup the Crow external library in this repo for creation of web app ui for client"
    echo "========================================================================================================================="
    echo "How to use:" 
    echo "  To Start: ./Crow.sh [flags]"
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
crowDir="${externDir}/crow"
if [[ ${mode} == "install" ]]; then

    # install gpio lib, have to be in the correct dir for this (chain commands)
    echo "=============== Bulding and Installing Crow Library! ==============="
    # following these instructions: https://github.com/ipkn/crow#building-tests-examples
    cd "${crowDir}" && \
        mkdir -p build && \
        cd build && \
        cmake .. && \
        make || \
        echo "Failed to build: try running ./install/helpers/linux_pkgs.sh to get requirements"

else # clean
    echo "=============== Cleaning Crow Library! ==============="
    cd "${crowDir}" && \
        rm -rf build || \
        echo "Failed to clean crow"
fi