#!/usr/bin/python3

# standard includes
import os
import sys
import gpiozero
from gpiozero import LED, Button, PWMLED
import signal
import time
from time import sleep

class ButtonLedPair():
    def __init__(self, LEDobj:PWMLED, buttonObj:Button):
        """
            Helper class that turns on a specific LED based on a specific button

            \nArgs - LEDobj (PWMLED): The created PWMLed object to turn on
            \nArgs - buttonObj (Button): The created Button object to control the LED
        """

        super().__init__()
        self.ledToControl = LEDobj
        self.btnController = buttonObj


    def buttonToLED(self):
        """ Runner function that turns on a specific LED based on a specific button

            \nNotes:
                Function wraps in an infinite loop so use a Thread to not block
        """

        while True:
            if self.btnController.is_pressed:
                self.ledToControl.on()
            else:
                self.ledToControl.off()