# Description

This repo contains both C++ and Python code for interfacing with a Raspberry Pi 3B+.
In the future however, I plan on stop supporting the Python code base in favor of C++.
_Note:_ I may rever to the raspberry pi as RPI for short.

## Current Features

Currently both C++ & Python codes allow for three main features:

1. Blink the LEDs at a given interval
2. Gradually increase the brightness of the LEDs until they reach the maximum and restart
3. Controll the LEDs using their corresponding buttons.

Use `./main.py --help` or `./bin/rpi_driver --help` to learn how to use it.

## Installing

For python no extra modules are currently required.
For C++ you will need to run the install script found at `./install/setup.sh`.

## Building

To build the C++ version of the code simply run `make` or `make release` (for optimized build).
The resulting executable should be found at `./bin/rpi_driver`.

## Issues

In order for build to not freeze on RPI, you might need to increase the available swap size. You can use `htop` to monitor Processors, Memory, and Swap utilization during a build.
Follow [this guide](https://pimylifeup.com/raspberry-pi-swap-file/) for how to do this. If some swap is not released after killing, completing, or stopping a build use the following commands to do it manually:

``` bash
sudo dphys-swapfile swapoff # turn swap off
sudo dphys-swapfile swapon  # turn swap back on
```

## Usage

Check out [this guide](https://github.com/NRizzoInc/RaspberryPi/wiki) for how to use the server & client to control the RPI.

## Setup

### GPIO Configuration

See the `PinDiagrams` directory gain an understanding of my RPI's pin usage.

_Note: the C++ code utilizes the `wPi` column in the `./PinDiagrams/README.md` file_

| wPi Pins 	| LEDs 	| Buttons 	|
|:--------:	|:----:	|:-------:	|
|    Red   	|   22  |    26    	|
|  Yellow  	|   23  |    27    	|
|   Green  	|   24  |    28    	|
|   Blue   	|   25  |    29    	|

### Software Configuration

Make sure all RPI interfaces are enabled (except "1-wire").
Click on raspberry pi symbol on top left -> "Preferences" -> "Raspberry Pi Configuration"

