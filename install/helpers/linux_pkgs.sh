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
    pkg-config


# camera packages
apt install -y \
    libx264-dev \
    libgtk-3-dev \
    libavcodec-dev libavformat-dev libswscale-dev libv4l-dev \
    libxvidcore-dev libjpeg-dev libpng-dev libtiff-dev \
    gfortran openexr libatlas-base-dev \
    libtbb2 libtbb-dev libdc1394-22-dev \
    python3-dev python3-numpy


apt upgrade -y

################################# fix issues with installed packages ###################################

# bug with mesa-common-dev not installing libGL.so correctly (need to manually create symlink)
# https://github.com/RobotLocomotion/drake/issues/2087#issue-148166827
ln -s /usr/lib/x86_64-linux-gnu/libGL.so.1 /usr/lib/x86_64-linux-gnu/libGL.so

echo "=============== Compelted Linux Package Manager Script ==============="
