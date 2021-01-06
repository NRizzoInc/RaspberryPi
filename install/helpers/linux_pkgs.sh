#!/usr/bin/bash
# File: Helps install all required linux packages

echo "================ Running Linux Package Manager Script ================"

# need to be sudo
if [ "$EUID" -ne 0 ]; then
    echo "Please run as root ('sudo')"
    exit
fi

apt update -y

apt install -y \
    cmake \
    gcc-7 g++-7 \
    mesa-common-dev \
    build-essential \
    libx264-dev \
    libopencv-dev \

apt upgrade -y

################################# fix issues with installed packages ###################################

# bug with mesa-common-dev not installing libGL.so correctly (need to manually create symlink)
# https://github.com/RobotLocomotion/drake/issues/2087#issue-148166827
ln -s /usr/lib/x86_64-linux-gnu/libGL.so.1 /usr/lib/x86_64-linux-gnu/libGL.so

echo "=============== Compelted Linux Package Manager Script ==============="
