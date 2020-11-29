#!/usr/bin/python3

# standard includes
from gpiozero import LED, Button, PWMLED
import signal
import time
from threading import Thread, Event
import argparse

# our includes
from threadHelpers.killableThreads import threadWithException, stopThreadOnSetCallback
from threadHelpers.ThreadHelperFunctions import startAndJoinThread
from ButtonLedController import ButtonLedPair

class GPIOBase():
    def __init__(self):
        super().__init__()

        # setup board objects
        self._setupPins()

        # Map functions based on the input mode
        self.__modeToAction = {
            "Blink"         : self.blinkLed,
            "LEDs"          : lambda ledObj, interval, *args, **kwargs: self.LEDIntensity(ledObj, interval),
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
        createPairs = lambda name: (name, ButtonLedPair(self.__leds[name], self.__btns[name]))
        self.btnLedPairs = dict(map(createPairs, list(self.__leds.keys())))

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

    def blinkLed(self, ledObj:PWMLED, interval:int=1, *args, **kwargs):
        """Blinks a specific LED

        \n@Args - LEDobj (PWMLED): The created PWMLed object to blink
        \n@Args - interval (int): How long (in seconds) the LED should stay on/off for
        \n@Notes:
            Function wraps in an infinite loop so use a Thread to not block
        """
        while True:
            # can also just use "blink(on_time, off_time)"
            ledObj.on()
            print("On")
            time.sleep(interval)
            ledObj.off()
            print("Off")
            time.sleep(interval)

    def LEDIntensity(self, ledObj:PWMLED, interval:int=1, *args, **kwargs):
        """
            Blinks LED but at increases intensity
            
            \n@Args - LEDobj (PWMLED): The created PWMLed object to blink
            \n@Args - interval (int): How long (in seconds) the LED should stay on/off for
            \n@Notes:
                Function wraps in an infinite loop so use a Thread to not block
        """
        brightness = 0
        while True:
            ledObj.value = brightness
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

    def run(self, mode:str, nameLi:list=[], interval=1, stopEvent:Event=None):
        """
            Run the desired operational mode based on the input
            \n@Args - mode (str): The GPIO mode to run (comes from getModeList())
            \n@args - nameLi (list): Contains names of all Button-Led Pairs to control
            \n@args - interval (float): seconds intrerval between led blinks
        """
        if mode not in self.getModeList(): raise Exception("Input mode does not exist!")

        # run function based on mode
        self.__modeToAction[mode](nameLi=nameLi, stopEvent=stopEvent, interval=interval)

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
    )
    
    parser.add_argument(
        "--interval",
        type=float,
        required=False,
        help="The interval to blink the LEDs",
    )

    # actually parse flags
    args = parser.parse_args()
    
    # call entered function
    gpioHandler.run(args.mode, nameLi=args.nameLi, interval=args.interval)

