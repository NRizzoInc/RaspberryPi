#!/bin/bash
#@file: Builds and Installs External RPI Camera Library

# CLI Flags
print_flags () {
    echo "========================================================================================================================="
    echo "Usage: camera.sh"
    echo "========================================================================================================================="
    echo "Helper utility to setup the RPI Camera external library in this repo to stream video to client"
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
cameraExternDir="${externDir}/camera"
raspicamDir="${cameraExternDir}/raspicam"

if [[ ${mode} == "install" ]]; then

    echo "============== Bulding and Installing Raspicam Library! =============="
    cd "${raspicamDir}" && \
        mkdir -p build && \
        cd build && \
        cmake .. && \
        make && \
        make install \
            || echo "Failed to build: try running ./install/helpers/linux_pkgs.sh to get requirements" \
            && ldconfig

else # clean
    echo "===================== Cleaning Raspicam Library! ====================="
    cd "${raspicamDir}" && \
        rm -rf build || \
        echo "Failed to clean Raspicam"
fi
