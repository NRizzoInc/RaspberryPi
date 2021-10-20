# Description

This repo contains both C++ and Python code for interfacing with a Raspberry Pi 3B+.
In the future however, I plan on stop supporting the Python code base in favor of C++.
_Note:_ I may refer to the raspberry pi as RPI for short.

## Current Features

### Main Features

To fully run the robot's systems, both the client and server need to be used.

1. First start up the server: `./bin/rpi_driver --mode server`
2. Then start up the client: `./bin/rpi_driver --mode client --ip <RPI's ip address>`

_Note:_ Using the verbose flag (`--verbose`) will show extra information in the terminal as the program is running.

## Usage

Check out [this guide](https://github.com/NRizzoInc/RaspberryPi/wiki) for how to use the server & client to control the RPI.

### Running Client on Another Device (not recommended)

You can run the client on another device, but you would need to build & install from source again and then determine the rpi's ip to connect to which is annoying (only Ubuntu is known to be supported, but I have no reason to believe it will not work on other Linux Distributions). Doing it this way make it slightly easier to access the web app since its address will be `http://127.0.0.1:<client port (default 5001)>/RPI-Client`, but is arguably more annoying because you would have to manage starting & stoppping the client.

### Running Client on RaspberryPi (recommended)

It is much simpler to actually run **both** the client & server on the RPI without specifying an ip (defaults to **localhost**). This in turn starts up a web app client which can be accessed by any device on the same network as the RPI. If you follow [this guide for setting up a hostname](https://www.howtogeek.com/167195/how-to-change-your-raspberry-pi-or-other-linux-devices-hostname/), then accessing the web app from another computer is as easy as opening a browser and going to `http://<hostname>:<client port (default 5001)>/RPI-Client`.

### Features that can be run locally without the client

1. Blink the LEDs at a given interval: `--mode blink`
2. Gradually increase the brightness of the LEDs until they reach the maximum and restart: `--mode intensity`
3. Control the LEDs using their corresponding buttons: `--mode btns`
4. Test the robots capability to move in each direction: `--mode motors`
5. Test the robots capability to move the camera's servos: `--mode servos`
6. Test the robots capability to detect distances with the ultrasonic sensor: `--mode ultrasonic`
7. Test the robots capability to combine the ultrasonic sensor with the servos/motors to perform obstacle avoidance: `--mode obstacle`
8. Test the camera and save the latest frame to disk: `--mode camera`

Use `./main.py --help` or `./bin/rpi_driver --help` to learn how to use it.

(_Note:_ Most features are now only supported by the c++ produced binary)

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

## Setup

### GPIO Configuration

See the `PinDiagrams` directory gain an understanding of my RPI's pin usage.

_Note: the C++ code utilizes the `wPi` column in the [`./PinDiagrams/README.md`](./PinDiagrams/README.md) file_

| wPi Pins  | LEDs  | Buttons   |
|:--------: |:----: |:-------:  |
|    Red    |   22  |    26     |
|  Yellow   |   23  |    27     |
|   Green   |   24  |    28     |
|   Blue    |   25  |    29     |

### Software Configuration

Make sure all RPI interfaces are enabled (except "1-wire").
Click on raspberry pi symbol on top left -> "Preferences" -> "Raspberry Pi Configuration"

### Final Appearance

![image.png](https://images.zenhubusercontent.com/5f0e67368238228fc09554ca/393abaf8-8595-44c4-8e1d-608d4d057b1f)
