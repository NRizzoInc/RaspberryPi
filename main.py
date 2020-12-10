#!/usr/bin/python3
"""Wrapper for python GPIO code"""

# standard includes
import argparse

# our includes
from src.python.GPIOBase import GPIOBase

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
        "--colors",
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
    gpioHandler.run(args.mode, args.nameLi, args.interval)