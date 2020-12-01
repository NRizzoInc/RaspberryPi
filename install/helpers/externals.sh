#!/bin/bash
#@file: Builds and Installs externals

# CLI Flags
print_flags () {
    echo "========================================================================================================================="
    echo "Usage: setup.sh"
    echo "========================================================================================================================="
    echo "Helper utility to setup everything to use this repo"
    echo "========================================================================================================================="
    echo "How to use:" 
    echo "  To Start: ./setup.sh [flags]"
    echo "========================================================================================================================="
    echo "Available Flags:"
    echo "    -i | --extern-dir: Path to the extern dir"
    echo "    -m | --mode: [install, clean, uninstall]"
    echo "    -h | --help: This message"
    echo "========================================================================================================================="
}

# parse command line args
mode=""
externDir=""
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -i | --extern-dir )
            externDir="$2"
            shift
            ;;
        -m | --mode )
            mode="$2"
            if [[ ${mode} == "install" ]]; then
                # ./build "" for regular building
                mode=""
            elif [[ ${mode} != "clean" && ${mode} != "uninstall" ]]; then
                echo "Mode can be [install, clean, uninstall]"
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
gpioLib="${externDir}/WiringPi"
gpioBuildExe="${gpioLib}/build"

# build/install libraries


# install gpio lib, have to be in the correct dir for this (chain commands)
echo "=============== Bulding and Installing GPIO Library! ==============="
cd "${gpioLib}" && \
    echo "cd-ed to GPIO Lib: ${gpioLib}" && \
    bash "${gpioBuildExe}" "${mode}"
