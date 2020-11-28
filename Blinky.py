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


# calls the correct function based on the mode
def main(mode:int):
    if mode == 1: justLED()
    if mode == 2: LEDIntensity()
    if mode == 3: redPress()
    if mode == 4: rgbyButtons()

# create lmbda specific function for each
redPress    = lambda: buttonToLED(redLED, redButton)
yellowPress = lambda: buttonToLED(yellowLED, yellowButton)
greenPress  = lambda: buttonToLED(greenLED, greenButton)
bluePress   = lambda: buttonToLED(blueLED, blueButton)

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



if __name__ == "__main__":    
    if len(sys.argv) < 2:
        print("Invalid # of arguments! (enter mode '1' for blink, or '2' for button control")
        sys.exit()
        
    mode = int(sys.argv[1])

    main(mode)
