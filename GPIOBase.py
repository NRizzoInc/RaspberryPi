#!/usr/bin/python3

# standard includes
import sys
from gpiozero import LED, Button, PWMLED
import signal
import time
from threading import Thread, Event
import argparse

# our includes
from threadHelpers.killableThreads import threadWithException, stopThreadOnSetCallback
from threadHelpers.SignalHelpers import signal_handler_generator, setup_sig_handler
from threadHelpers.ThreadHelperFunctions import startAndJoinThread
from ButtonLedController import ButtonLedPair

class GPIOBase():
    def __init__(self):
        super().__init__()

        # setup board objects
        self._setupPins()

        # Map functions based on the input mode
        self.__modeToAction = {
            "Blink"         : lambda nameLi, interval, stopEvent, *args, **kwargs: self.blinkLeds(nameLi, interval, stopEvent),
            "Intensity"     : lambda nameLi, interval, *args, **kwargs: self.LEDIntensity(nameLi, interval),
            "Btns"          : lambda nameLi, *args, **kwargs: self.handleLedBtns(nameLi, *args, **kwargs),
            "All-Btns"      : self.runAllLedBtns
        }

    def _setupPins(self):
        """Sets up the board based on my breadboard's pin configuration"""
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

        #----------------------------------------- LCD(4 bit mode) -----------------------------------------#
        self.__LCD = {
            "pin4"  : 36,  # LCD #4  = GPIO16 (=pin36) 
            "pin6"  : 32,  # LCD #6  = GPIO12 (=Pin32)  
            "pin11" : 1,  # LCD #11 = GPIO1  (=Pin28)
            "pin12" : 7,  # LCD #12 = GPIO7  (=Pin26)
            "pin13" : 8,  # LCD #13 = GPIO8  (=Pin24)
            "pin14" : 25, # LCD #14 = GPIO25 (=Pin22)
        }

    def getBtnLedPairNames(self)->list:
        """Returns a list of all led-button pair names"""
        return list(self.btnLedPairs.keys())

    def getModeList(self)->list:
        """Returns a list of all available modes"""
        return list(self.__modeToAction.keys())

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

    def setupBtnLedControl(self, pairName:str, stopEvent:Event=None, *args, **kwargs)->threadWithException:
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
            stopEvent=stopEvent
        )

    def blinkLeds(self, nameLi:list, interval:int, stopEvent:Event=None, *args, **kwargs)->threadWithException:
        """
            Blinks a specific LED

            \n@Args - nameLi ([str...]): List of names that map to the PWMLed objects to blink with diff intensity
            \n@Args - interval (int): How long (in seconds) the LED should stay on/off for
            \n@Returns - thread running the blinker
        """
        # figure out which ones to blink
        toBlinkLi = [name for name in nameLi if name in self.getLedNames()]
        toBlinkStr = ", ".join([name for name in toBlinkLi])
        print(f"Blinking: " + toBlinkStr)
        print(f"interval: {interval}s")

        def blinkWorker():
            isOn = False
            while True:
                for ledName in toBlinkLi:
                    ledObj = self.getLedObj(ledName)
                    if  isOn:   ledObj.off()
                    else:       ledObj.on()

                isOn = not isOn
                time.sleep(interval)

        blinkThread = threadWithException(
            name="Blink-Thread",
            target=blinkWorker,
            toPrintOnStop=f"Stopping Blinker Thread",
            stopEvent=stopEvent
        )
        blinkThread.start()
        return blinkThread


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

    def handleLedBtns(self, liPairs:list=[], *args, **kwargs):
        """
            Handles the starting, joining, and ending of the LED-Button threads

            \n@Args - liPairs (list): List of names for ButtonLedPair objects to handle
            \n@Note: Blocks main thread
        """
        # create a process for each color
        # add an event which is used to communicate stopping to all threads
        stopEvent = Event()
        threadList = []

        # add all threads to a list to start at the same time
        for LedBtnPairName in liPairs:
            if LedBtnPairName not in self.getBtnLedPairNames():
                raise Exception(f"Button pair {LedBtnPairName} does not exist!")

            threadList.append(self.setupBtnLedControl(LedBtnPairName, stopEvent))
            
        # start all the threads
        for proc in threadList:
            proc.start()

        # end threads via control+c
        # end all threads via raise_exception
        # cleanup with join just in case
        def signal_handler(sig, frame):
            print('You pressed Ctrl+C')
            for proc in threadList:
                proc.raise_exception()
                proc.join()

        # actually create control+c handler
        print("Press Ctrl+C to Stop")
        signal.signal(signal.SIGINT, signal_handler)
        signal.pause()

    def runAllLedBtns(self, *args, **kwargs):
        """
            Controls all LEDs controlled by button (red, yellow, green blue)- using threads
            Function can be stopped using Ctrl+C

            \n@Note: Blocks main thread
        """
        # collate list of all button pairs
        self.handleLedBtns(list(self.btnLedPairs.keys()))

    def run(self, mode:str, nameLi:list, interval, stopEvent:Event=None):
        """
            Run the desired operational mode based on the input
            \n@Args - mode (str): The GPIO mode to run (comes from getModeList())
            \n@args - nameLi (list): Contains names of all Button-Led Pairs to control
            \n@args - interval (float): seconds intrerval between led blinks
        """
        if mode not in self.getModeList(): raise Exception("Input mode does not exist!")

        # run function based on mode
        return self.__modeToAction[mode](nameLi=nameLi, stopEvent=stopEvent, interval=interval)

if __name__ == "__main__":
    # create gpio handler obj
    gpioHandler = GPIOBase()

    # command CLI args
    parser = argparse.ArgumentParser(description="Basic GPIO Controller")
    parser.add_argument(
        "-m", "--mode",
        required=True,
        help="Which action to perform",
        choices=list(gpioHandler.getModeList())
        
    )

    parser.add_argument(
        "--names",
        type=str,
        nargs="+", # accept multiple args and store in list
        required=False,
        help="Which Led-Button Pairs (multiple) to use. Space seperated",
        choices=list(gpioHandler.getBtnLedPairNames()),
        dest="nameLi",
        default=[]
    )
    
    parser.add_argument(
        "--interval",
        type=float,
        required=False,
        help="The interval to blink the LEDs",
        default=1
    )

    # actually parse flags
    args = parser.parse_args()

    # call entered function
    stopEvent = Event()
    thread = gpioHandler.run(args.mode, args.nameLi, args.interval, stopEvent)

    # setup handlers to gracefully end thread created by run
    setup_sig_handler(thread)
