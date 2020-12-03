#!/usr/bin/python3
"""@File: Manages all the raspberry pi's GPIO functions (LEDs, Buttons, LCDs)"""

# standard includes
import sys
import signal
import time
from threading import Event
import argparse

# 3rd party includes
from gpiozero import LED, Button, PWMLED

# our includes
from .threadHelpers.killableThreads import threadWithException
from .threadHelpers.SignalHelpers import setup_sig_handler
from .ButtonLedController import ButtonLedController

class GPIOBase(ButtonLedController):
    def __init__(self):
        # call init functions to setup pins
        super().__init__()

        # setup board objects
        self._setupPins()

        # Map functions based on the input mode
        self.__modeToAction = {
            "Blink"         : lambda nameLi, interval, *args, **kwargs: self.blinkLeds(nameLi, interval),
            "Intensity"     : lambda nameLi, interval, *args, **kwargs: self.LEDIntensity(nameLi, interval),
            "Btns"          : lambda nameLi, stopEventLoop, *args, **kwargs: self.handleLedBtns(nameLi, stopEventLoop),
            "All-Btns"      : self.runAllLedBtns
        }

    def _setupPins(self):
        """Sets up the board based on my breadboard's pin configuration"""

        #----------------------------------------- LCD(4 bit mode) -----------------------------------------#
        self.__LCD = {
            "pin4"  : 36,  # LCD #4  = GPIO16 (=pin36) 
            "pin6"  : 32,  # LCD #6  = GPIO12 (=Pin32)  
            "pin11" : 1,  # LCD #11 = GPIO1  (=Pin28)
            "pin12" : 7,  # LCD #12 = GPIO7  (=Pin26)
            "pin13" : 8,  # LCD #13 = GPIO8  (=Pin24)
            "pin14" : 25, # LCD #14 = GPIO25 (=Pin22)
        }

    def getModeList(self)->list:
        """Returns a list of all available modes"""
        return list(self.__modeToAction.keys())

    def run(self, mode:str, nameLi:list, interval:float, stopEvent:Event=None):
        """
            Run the desired operational mode based on the input
            \n@Args - mode (str): The GPIO mode to run (comes from getModeList())
            \n@args - nameLi (list): Contains names of all Button-Led Pairs to control
            \n@args - interval (float): seconds intrerval between led blinks
            \n@args - stopEvent (threading.Event): An event to stop threads (optional)
        """
        if mode not in self.getModeList(): raise Exception("Input mode does not exist!")
        stopEvent = stopEvent if stopEvent != None else Event()

        # run function based on mode
        runThread = threadWithException(
            name=f"{mode}-Thread",
            target=self.__modeToAction[mode],
            toPrintOnStop=f"Stopping {mode} Thread",
            # additional params
            nameLi=nameLi,
            interval=interval,
            stopEvent=stopEvent,
            stopEventLoop=stopEvent,
        )
        # setup handlers to gracefully end created thread
        setup_sig_handler(runThread, stopEvent)

        # start the thread
        runThread.start()
        runThread.join()
