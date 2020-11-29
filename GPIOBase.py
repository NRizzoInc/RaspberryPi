#!/usr/bin/python3

# standard includes
from gpiozero import LED, Button, PWMLED
import signal
import time
from threading import Thread, Event
import argparse

# our includes
from threadHelpers.killableThreads import threadWithException, stopThreadOnSetCallback
from ButtonLedController import ButtonLedPair

class GPIOBase():
    def __init__(self):
        super().__init__()

        # setup board objects
        self._setupPins()

        # Map functions based on the input mode
        self.__modeToAction = {
            "Blink"     :   self.blinkLed,
            "LEDs"      :   self.LEDIntensity,
            "Red-Btn"   :   self.btnLedPairs["red"].buttonToLED,
            "Btns"      :   self.startBtnLedControl
        }

    def _setupPins(self):
        """Sets up the board based on my breadboard's pin configuration"""
        #------------------------------------------ Buttons & LED ------------------------------------------#
        self.__redLED         =  PWMLED(24) # gpio24/ pin 18 (can use variable brightness)
        self.__yellowLED      =  PWMLED(23)
        self.__greenLED       =  PWMLED(18)
        self.__blueLED        =  PWMLED(15)
        self.__redButton      =  Button(2) # "gpio2" = "BOARD3"
        self.__yellowButton   =  Button(3)
        self.__greenButton    =  Button(4)
        self.__blueButton     =  Button(17)

        #------------------------------------------ Btn-LED Pairs ------------------------------------------#
        self.btnLedPairs = {
            "red"     : ButtonLedPair(self.__redLED, self.__redButton),
            "yellow"  : ButtonLedPair(self.__yellowLED, self.__yellowButton),
            "green"   : ButtonLedPair(self.__greenLED, self.__greenButton),
            "blue"    : ButtonLedPair(self.__blueLED, self.__blueButton)
        }

        #----------------------------------------- LCD(4 bit mode) -----------------------------------------#
        self.pin4 = 36  # LCD #4  = GPIO16 (=pin36) 
        self.pin6 = 32  # LCD #6  = GPIO12 (=Pin32)  
        self.pin11 = 1  # LCD #11 = GPIO1  (=Pin28)
        self.pin12 = 7  # LCD #12 = GPIO7  (=Pin26)
        self.pin13 = 8  # LCD #13 = GPIO8  (=Pin24)
        self.pin14 = 25 # LCD #14 = GPIO25 (=Pin22)

    def getBtnLedPairNames(self)->list:
        """Returns a list of all led-button pair names"""
        return list(self.btnLedPairs.keys())

    def getModeList(self)->list:
        """Returns a list of all available modes"""
        return list(self.__modeToAction.keys())

    def startBtnLedControl(self, pairName:str, stopEvent:Event=None)->threadWithException:
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

    def blinkLed(self, ledObj:PWMLED, interval:int=1):
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

    def LEDIntensity(self, ledObj:PWMLED, interval:int=1):
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

    def runAllLedBtns(self):
        """
            Controls all LEDs controlled by button (red, yellow, green blue)- using threads
            Function can be stopped using Ctrl+C

            \n@Note: Blocks main thread
        """
        # create a process for each color (total of 4- red, yellow, green, blue)
        # add an event which is used to communicate stopping to all threads
        stopEvent = Event()
        
        threadList = []

        # add all threads to a list to start at the same time
        for LedBtnPairName in self.btnLedPairs.keys():
            threadList.append(self.startBtnLedControl(LedBtnPairName, stopEvent))
            
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

    def run(self, mode:str):
        """
            Run the desired operational mode based on the input
            \n@Args - mode (str): The GPIO mode to run (comes from getModeList())
        """
        if mode not in self.getModeList(): raise Exception("Input mode does not exist!")

        # run function based on mode
        self.__modeToAction[mode]()

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

    # actually parse flags
    args = parser.parse_args()
    
    # call entered function
    gpioHandler.run(args.mode)

