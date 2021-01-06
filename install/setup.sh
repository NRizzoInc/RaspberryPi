#!/bin/bash
#@file: Intended to call other helper scripts in order to install all neccessary components to compile

[[ "$OSTYPE" == "msys" || "$OSTYPE" == "cygwin" ]] && isWindows=true || isWindows=false

# if linux, need to check if using correct permissions
if [[ "${isWindows}" = false ]]; then
    if [ "$EUID" -ne 0 ]; then
        echo "Please run as root ('sudo')"
        exit
    fi
else # is windows
    echo "Windows install is not supported. Use a linux system or WSL..."
    exit
fi


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
    echo "Available Flags (mutually exclusive):"
    echo "    -a | --install-all: (Default) If used, install everything (recommended for fresh installs)"
    echo "    -p | --linux-pkgs: Install all the required linux packages"
    echo "    -s | --submodules: Fetch & Update all the git submodules in this repo"
    echo "    -g | --gpio: Build and Install the c++ gpio library from source"
    echo "    -w | --web: Build and Install the c++ web-app library \"pistache\" to capture keyboard input for client"
    echo "    -c | --camera: Build and Install the c++ camera library to stream video to the client"
    echo "    -h | --help: This message"
    echo "========================================================================================================================="
}

# parse command line args
installGPIO=false
installPistache=false
installCamera=false
updateSubmodules=false
linuxPkgs=false
installAll=true # default to installing everything
while [[ "$#" -gt 0 ]]; do
    case $1 in
        -a | --install-all )
            echo "Installing all externals!"
            break
            ;;
        -p | --linux-pkgs )
            linuxPkgs=true
            installAll=false
            break
            ;;
        -g | --gpio )
            installGPIO=true
            installAll=false
            break
            ;;
        -w | --web )
            installPistache=true
            installAll=false
            break
            ;;
        -c | --camera )
            installCamera=true
            installAll=false
            break
            ;;
        -s | --submodules )
            updateSubmodules=true
            installAll=false
            break
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

# Get absolute paths
INSTALL_DIR="$(readlink -fm "$0"/..)"
rootDir="$(readlink -fm "${INSTALL_DIR}/..")"
externDir="${rootDir}/extern"
helpersDir="${INSTALL_DIR}/helpers"

# Get helper script paths
wiringPiScript="${helpersDir}/WiringPi.sh"
submoduleScript="${helpersDir}/submodules.sh"
pistacheScript="${helpersDir}/pistache.sh"
cameraScript="${helpersDir}/camera.sh"
linuxPkgsScript="${helpersDir}/linux_pkgs.sh"

# call helpers as needed
echo "========== Calling Helper Scripts ==========="

# linux packages
if [[ ${linuxPkgs} == true || ${installAll} == true ]]; then
    bash "${linuxPkgsScript}"
fi

# git submodules script
if [[ ${updateSubmodules} == true || ${installAll} == true ]]; then
    bash "${submoduleScript}" \
        --root-dir "${rootDir}"
fi

# external scripts
if [[ ${installGPIO} == true || ${installAll} == true ]]; then
    bash "${wiringPiScript}" \
        --extern-dir "${externDir}"
fi

if [[ ${installPistache} == true || ${installAll} == true ]]; then
    bash "${pistacheScript}" \
        --extern-dir "${externDir}"
fi

if [[ ${installCamera} == true || ${installAll} == true ]]; then
    bash "${cameraScript}" \
        --extern-dir "${externDir}"
fi

echo "========= Completed Helper Scripts ========="


echo "Setup Complete!!"
