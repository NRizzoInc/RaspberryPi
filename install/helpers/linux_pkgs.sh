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

apt upgrade -y

echo "=============== Compelted Linux Package Manager Script ==============="
