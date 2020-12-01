#!/bin/bash
#@file: Fetches and updates this repo's submodules

# CLI Flags
print_flags () {
    echo "========================================================================================================================="
    echo "Usage: submodules.sh"
    echo "========================================================================================================================="
    echo "Helper utility to setup/update git submodules in this repo"
    echo "========================================================================================================================="
    echo "How to use:" 
    echo "  To Start: ./submodules.sh [flags]"
    echo "========================================================================================================================="
    echo "Available Flags:"
    echo "    --root-dir <dir>: Absolute path to the root of the repo"
    echo "    -h | --help: This message"
    echo "========================================================================================================================="
}

# parse command line args
rootDir=""
while [[ "$#" -gt 0 ]]; do
    case $1 in
        --root-dir )
            rootDir="$2"
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

# best to handle submodules from root
echo "========== Updating Git Submodules ==========="
cd "${rootDir}" || echo "Failed to cd to repo root: ${rootDir}" && \
    git submodule init && \
    git submodule update
