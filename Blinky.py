#!/usr/bin/python3

import os
import sys
import gpiozero
from gpiozero import LED, Button, PWMLED
from signal import pause
import time
from time import sleep
import multiprocessing
from multiprocessing import Process
import threading
from threading import Thread
import argparse

# setup board objects
redLED = PWMLED(24) # gpio18 (can use variable brightness)
yellowLED = PWMLED(23)
greenLED = PWMLED(18)
blueLED = PWMLED(15)
redButton = Button(2) # "gpio2" = "BOARD3"
yellowButton = Button(3)
greenButton = Button(4)
blueButton = Button(17)


# helper function that turns on a specific LED based on a specific button
def buttonToLED(LEDobj:PWMLED, buttonObj:Button):
    while True:
        if buttonObj.is_pressed:
            LEDobj.on()
        else:
            LEDobj.off()

# blinks LED
def justLED():
    while True:
        # can also just use "blink(on_time, off_time)"
        redLED.on()
        time.sleep(1)
        redLED.off()
        time.sleep(1)

# blinks LED but increases intensity
def LEDIntensity():
    brightness = 0
    while True:
        redLED.value = brightness
        time.sleep(1)
        brightness = (brightness + .5) % 1.5 # three settings 0, .5, 1 

# blinks with button (red, yellow, green blue)- uses multiprocess
def rgbyButtons():
    # create a process for each color (total of 4- red, yellow, green, blue)
    redProc = Thread(target=redPress, name='red')
    yellowProc = Thread(target=yellowPress, name='yellow')
    greenProc = Thread(target=greenPress, name='green')
    blueProc = Thread(target=bluePress, name='blue')

    # start the processes
    redProc.start()
    yellowProc.start()
    greenProc.start()
    blueProc.start()



# create lmbda specific function for each
redPress    = lambda: buttonToLED(redLED, redButton)
yellowPress = lambda: buttonToLED(yellowLED, yellowButton)
greenPress  = lambda: buttonToLED(greenLED, greenButton)
bluePress   = lambda: buttonToLED(blueLED, blueButton)

# calls the correct function based on the mode
modeToAction = {
    "LED-Blink"         :   justLED(),
    "LED-Intensity"     :   LEDIntensity(),
    "Red-Button"        :   redPress(),
    "Buttons"           :   rgbyButtons()
}


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description="Basic GPIO Controller")
    parser.add_argument(
        "-m", "--mode",
        required=True,
        help="Which action to perform",
        choices=list(modeToAction.keys())
        
    )

    # actually parse flags
    args = parser.parse_args()
    
    # call entered function
    modeToAction[args.mode]()
