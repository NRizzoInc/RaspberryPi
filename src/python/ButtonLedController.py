#!/usr/bin/python3
"""@File: Module handles all things relating to the Buttons & LEDs"""

# standard includes
from time import sleep
from threading import Event
from .threadHelpers.killableThreads import threadWithException

# standard includes
import time

# our includes
from .ButtonLedPair import ButtonLedPair

# 3rd Party Includes
from gpiozero import Button, PWMLED

class ButtonLedController():
    def __init__(self):
        """Class that manages the Buttons and LEDs on the GPIO pins"""
        # declare vars to be defined later
        self.__leds = None
        self.__btns = None
        self.btnLedPairs = {}

        self._setupBtnLedPins()

    def _setupBtnLedPins(self):
        """Sets up the board's lcd & led based on my breadboard's pin configuration"""
        #------------------------------------------ Buttons & LED ------------------------------------------#
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

        #------------------------------------------ Btn-LED Pairs ------------------------------------------#
        createPairs = lambda name: (name, ButtonLedPair(self.getLedObj(name), self.getBtnObj(name)))
        self.btnLedPairs = dict(map(createPairs, list(self.getLedNames())))

    def getLedNames(self)->list:
        """Returns a list of all led names"""
        return list(self.__leds.keys())

    def getBtnNames(self)->list:
        """Returns a list corresponding to all the btn names"""
        return list(self.__btns.keys())

    def getLedObj(self, ledName:str)->PWMLED:
        """Returns the led obj corresponding to the name"""
        return self.__leds[ledName]

    def getBtnObj(self, ledName:str)->Button:
        """Returns the btn obj corresponding to the name"""
        return self.__btns[ledName]

    def getBtnLedPairNames(self)->list:
        """Returns a list of all led-button pair names"""
        return list(self.btnLedPairs.keys())

############################################## Actual Functions ##############################################

    def blinkLeds(self, nameLi:list, interval:int, *args, **kwargs):
        """
            Blinks a specific LED

            \n@Args - nameLi ([str...]): List of names that map to the PWMLed objects to blink with diff intensity
            \n@Args - interval (int): How long (in seconds) the LED should stay on/off for
        """
        # figure out which ones to blink
        toBlinkLi = [name for name in nameLi if name in self.getLedNames()]
        toBlinkStr = ", ".join([name for name in toBlinkLi])
        print(f"Blinking: " + toBlinkStr)
        print(f"interval: {interval}s")

        isOn = False
        while True:
            for ledName in toBlinkLi:
                ledObj = self.getLedObj(ledName)
                if  isOn:   ledObj.off()
                else:       ledObj.on()

            isOn = not isOn
            time.sleep(interval)

    def LEDIntensity(self, nameLi:list, interval:int, *args, **kwargs):
        """
            Blinks LED but at increases intensity
            
            \n@Args - nameLi ([str...]): List of names that map to the PWMLed objects to blink with diff intensity
            \n@Args - interval (int): How long (in seconds) the LED should stay on/off for
            \n@Notes:
                Function wraps in an infinite loop so use a Thread to not block
        """
        # figure out which ones to change brightness for
        toBlinkLi = [name for name in nameLi if name in self.getLedNames()]
        toBlinkStr = ", ".join([name for name in toBlinkLi])
        print(f"Changing intensity for: " + toBlinkStr)
        print(f"interval: {interval}s")
        
        brightness = 0
        while True:
            # for each listed led, change intensity
            for ledName in toBlinkLi:
                self.getLedObj(ledName).value = brightness 
            time.sleep(interval)
            brightness = (brightness + .5) % 1.5 # three settings 0, .5, 1

############################################## Thread Functions ##############################################

    def setupBtnLedControl(self, pairName:str, stopEvent:Event=None)->threadWithException:
        """Start a thread in which the led is controlled by the button

        \n@Args - pairName (str): The name of the LED-Button Pair (found with getBtnLedPairNames)
        \n@Args - stopEvent (threading.Event): optional arg used to communicate between threads

        \n@Returns - threadWithException: A special thread that can be killed with ".raise_exception()".
            None if invalid pairName
        """
        if pairName not in self.getBtnLedPairNames(): return None

        return threadWithException(
            target=self.btnLedPairs[pairName].buttonToLED,
            toPrintOnStop=f"Stopping {pairName}",
            name=pairName,
            stopEvent=stopEvent,
            stopEventLoop=stopEvent
        )

    def handleLedBtns(self, liPairs:list, stopEvent:Event):
        """
            Handles the starting, joining, and ending of the LED-Button threads

            \n@Args - liPairs (list): List of names for ButtonLedPair objects to handle
        """
        # create a process for each color
        # add an event which is used to communicate stopping to all threads
        threadList = []

        # add all threads to a list to start at the same time
        for LedBtnPairName in liPairs:
            if LedBtnPairName not in self.getBtnLedPairNames():
                raise Exception(f"Button pair {LedBtnPairName} does not exist!")

            threadList.append(self.setupBtnLedControl(LedBtnPairName, stopEvent))
            
        # start all the threads
        for proc in threadList:
            print(f"Starting {proc.getName()}")
            proc.start()

        stopEvent.wait()

        # stop threads since stop event is complete
        for proc in threadList:
            proc.raise_exception()

        return threadList

    def runAllLedBtns(self, stopEventLoop:Event, *args, **kwargs):
        """
            Controls all LEDs controlled by button (red, yellow, green blue)- using threads
            Function can be stopped using Ctrl+C

            \n@Note: Blocks main thread
        """
        # collate list of all button pairs
        self.handleLedBtns(list(self.btnLedPairs.keys()), stopEventLoop)
