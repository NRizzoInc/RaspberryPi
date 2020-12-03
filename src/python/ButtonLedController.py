#!/usr/bin/python3
"""@File: Module handles all things relating to the Buttons & LEDs"""

# standard includes
from time import sleep
from threading import Event

# our includes
from .ButtonLedPair import ButtonLedPair

# 3rd Party Includes
from gpiozero import LED, Button, PWMLED

class ButtonLedController():
    def __init__(self):
        """Class that manages the Buttons and LEDs on the GPIO pins"""
        self.__leds = {
            "red"     : PWMLED(24), # gpio24/ pin 18 (can use variable brightness)
            "yellow"  : PWMLED(23),
            "green"   : PWMLED(18),
            "blue"    : PWMLED(15)
        }
        self.__btns = {
            "red"     : Button(2), # "gpio2" = "BOARD3"
            "yellow"  : Button(3),
            "green"   : Button(4),
            "blue"    : Button(17)
        }
