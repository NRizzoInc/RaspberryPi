#!/bin/bash
#@file: Builds and Installs External pistache Library

# CLI Flags
print_flags () {
    echo "========================================================================================================================="
    echo "Usage: pistache.sh"
    echo "========================================================================================================================="
    echo "Helper utility to setup the pistache external library in this repo for creation of web app ui for client"
    echo "========================================================================================================================="
    echo "How to use:" 
    echo "  To Start: ./pistache.sh [flags]"
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
pistacheDir="${externDir}/pistache"
if [[ ${mode} == "install" ]]; then

    # install gpio lib, have to be in the correct dir for this (chain commands)
    echo "=============== Bulding and Installing Pistache Library! ==============="
    # following these instructions: http://pistache.io/quickstart#requirements
    cd "${pistacheDir}" && \
        mkdir -p build && \
        cd build && \
        cmake -G "Unix Makefiles" -DCMAKE_BUILD_TYPE=Release .. && \
        make || \
        echo "Failed to build: try running ./install/helpers/linux_pkgs.sh to get requirements"

else # clean
    echo "=============== Cleaning Pistache Library! ==============="
    cd "${pistacheDir}" && \
        rm -rf build || \
        echo "Failed to clean Pistache"
fi