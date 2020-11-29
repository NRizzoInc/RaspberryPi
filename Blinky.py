#!/usr/bin/python3

# standard includes
import os
import sys
import gpiozero
from gpiozero import LED, Button, PWMLED
from signal import pause
import time
from time import sleep
from threading import Thread, Event
import argparse

# our includes
from threadHelpers.killableThreads import threadWithException, stopThreadOnSetCallback

# setup board objects
redLED = PWMLED(24) # pin18 (can use variable brightness)
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
        print("On")
        time.sleep(1)
        redLED.off()
        print("Off")
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
    # add an event which is used to communicate stopping to all threads
    stopEvent = Event()
    
    redProc = threadWithException(
        target=redPress,
        toPrintOnStop="Stopping red",
        name='red',
        stopEvent=stopEvent
    )
    yellowProc = threadWithException(
        target=yellowPress, 
        toPrintOnStop="Stopping yellow",
        name='yellow',
        stopEvent=stopEvent
    )
    greenProc = threadWithException(
        target=greenPress,  
        toPrintOnStop="Stopping green",
        name='green',
        stopEvent=stopEvent
    )
    blueProc = threadWithException(
        target=bluePress,   
        toPrintOnStop="Stopping blue",
        name='blue',
        stopEvent=stopEvent
    )

    # start the processes
    redProc.start()
    redProc.join()

    yellowProc.start()
    yellowProc.join()

    greenProc.start()
    greenProc.join()

    blueProc.start()
    blueProc.join()

    # end all threads
    
    # stopEvent.set()



# create lmbda specific function for each
redPress    = lambda: buttonToLED(redLED, redButton)
yellowPress = lambda: buttonToLED(yellowLED, yellowButton)
greenPress  = lambda: buttonToLED(greenLED, greenButton)
bluePress   = lambda: buttonToLED(blueLED, blueButton)

# calls the correct function based on the mode
modeToAction = {
    "Red-Blink"         :   justLED,
    "LED-Intensity"     :   LEDIntensity,
    "Red-Button"        :   redPress,
    "Buttons"           :   rgbyButtons
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
