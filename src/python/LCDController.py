#!/usr/bin/python3
import os
import sys
import gpiozero
from gpiozero import LED, Button, PWMLED
from RPLCD import CharLCD # for LCD (had to sudo pip RPLCD)
from signal import pause
import time
from threading import Thread

#--------------Buttons & LED-------------------#
# setup board objects
redLED = PWMLED(24) # gpio24/ pin 18
yellowLED = PWMLED(23)
greenLED = PWMLED(18)
blueLED = PWMLED(15)
redButton = Button(2) # "gpio2" = "BOARD3"
yellowButton = Button(3)
greenButton = Button(4)
blueButton = Button(17)

#-------------LCD(4 bit mode)-------------#
pin4 = 36  # LCD #4  = GPIO16 (=pin36) 
pin6 = 32  # LCD #6  = GPIO12 (=Pin32)  
pin11 = 1  # LCD #11 = GPIO1  (=Pin28)
pin12 = 7  # LCD #12 = GPIO7  (=Pin26)
pin13 = 8  # LCD #13 = GPIO8  (=Pin24)
pin14 = 25 # LCD #14 = GPIO25 (=Pin22)

column = 
row = 
pins_rs = 
pins_e =
pins_loc = [] # LCD #11-14
lcd = CharLCD(column, row)


# calls the correct function based on the mode
def main(mode:int):
    if mode == 1: justLED()
    if mode == 2: LEDIntensity()
    if mode == 3: redPress()
    if mode == 4: rgbyButtons()
    if mode == 5: LCD()

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

# starts up the LCD with the correct pins
def LCD():
    



if __name__ == "__main__":    
    if len(sys.argv) < 2:
        print("Invalid # of arguments! (enter mode '1' for blink, or '2' for button control")
        sys.exit()
        
    mode = int(sys.argv[1])

    main(mode)
