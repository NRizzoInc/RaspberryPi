# Description

This repo contains both C++ and Python code for interfacing with a Raspberry Pi 3B+.
In the future however, I plan on stop supporting the Python code base in favor of C++. 

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

## GPIO Configuration

See the `PinDiagrams` directory gain an understanding of my RPI's pin usage.

_Note: the C++ code utilizes the `wPi` column in the `./PinDiagrams/README.md` file_

| wPi Pins 	| LEDs 	| Buttons 	|
|:--------:	|:----:	|:-------:	|
|    Red   	|   5  	|    8    	|
|  Yellow  	|   4  	|    9    	|
|   Green  	|   1  	|    7    	|
|   Blue   	|  16  	|    0    	|

