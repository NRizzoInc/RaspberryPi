#!/usr/bin/python3
"""@File: Manages all the raspberry pi's GPIO functions (LEDs, Buttons, LCDs)"""

# standard includes
from threading import Event

# our includes
from .threadHelpers.killableThreads import threadWithException
from .threadHelpers.SignalHelpers import setup_sig_handler
from .ButtonLedController import ButtonLedController
from .LCDController import LCDController

class GPIOBase(ButtonLedController, LCDController):
    def __init__(self):
        """Responsible for Managing all components of the GPIO (LED, LCD, Btns, Servos...)
        """
        # call init functions to setup pins
        # super().__init__()
        ButtonLedController.__init__(self)
        LCDController.__init__(self)

        # Map functions based on the input mode
        self.__modeToAction = {
            "Blink"         : lambda nameLi, interval, *args, **kwargs: self.blinkLeds(nameLi, interval),
            "Intensity"     : lambda nameLi, interval, *args, **kwargs: self.LEDIntensity(nameLi, interval),
            "Btns"          : lambda nameLi, stopEventLoop, *args, **kwargs: self.handleLedBtns(nameLi, stopEventLoop),
            "All-Btns"      : self.runAllLedBtns
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
