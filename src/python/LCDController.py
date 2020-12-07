#!/usr/bin/python3

# standard includes
import time
from threading import Thread

# 3rd party includes
import RPi.GPIO as GPIO # only available on linux distros (needed to set numbering_mode)
from RPLCD.gpio import CharLCD

# our includes

class LCDController():
    def __init__(self)->None:
        """Class that manages the LCD on the GPIO pins"""
        # https://rplcd.readthedocs.io/en/stable/getting_started.html
        super().__init__()
        print("In LCD")
        self.lcdObj = self._setupLCD()
        print("writting message")
        self.lcdObj.write_string("Hello World")

    def _setupLCD(self)->CharLCD:
        """Setup LCD and return the created object""" 
        #----------------------------------------- LCD(4 bit mode) -----------------------------------------#
        # config (https://pimylifeup.com/raspberry-pi-lcd-16x2/) -- using different scheme
        # LCD1                              = GND
        # LCD2                              = VCC+5
        # LCD3  (contrast adjustment)       = Potentiometer-output-middle 
        # LCD4  (H/L Reg Select Signal)     = 22 (GPIO 25) => 36 (GPIO 16)
        # LCD5  (H/L Read/Write Signal)     = GND 
        # LCD6  (Enable Signal)             = 18 (GPIO 24) => 32 (GPIO 12) 
        # LCD7  (Data Line Bus 0)           = Empty
        # LCD8  (Data Line Bus 1)           = Empty
        # LCD9  (Data Line Bus 2)           = Empty
        # LCD10 (Data Line Bus 3)           = Empty
        # LCD11 (Data Line Bus 4)           = 16 (GPIO 23) => 28 (GPIO 1)
        # LCD12 (Data Line Bus 5)           = 11 (GPIO 17) => 26 (GPIO 7) 
        # LCD13 (Data Line Bus 6)           = 12 (GPIO 18) => 24 (GPIO 8)
        # LCD14 (Data Line Bus 7)           = 15 (GPIO 22) => 22 (GPIO 25)
        # LCD15                             = VCC+5
        # LCD16                             = GND
        # Note: Depends on which numbering system being used (GPIO or than pin numbers)
        __LCDMapping = {
            # GPIO numbers
            "GPIO": {
                "numbering_mode":   GPIO.BCM,       # selects GPIO numbering system
                "pin_rs":           16,             # register select
                "pin_e":            12,             # enable (start read/write)
                "pin_rw":           21,             # (not actually used) read/write select
                "pin_backlight":    20,             # (not actually used) set backlight on/off 
                "pins_data":        [1, 7, 8, 25]   # list of data GPIO pins
            },
            # pin numberings
            "Pin": {
                "numbering_mode":   GPIO.BOARD,     # selects Pin numbering system
                "pin_rs":           36,             # register select
                "pin_e":            32,             # enable (start read/write)
                "pin_rw":           40,             # (not actually used) read/write select
                "pin_backlight":    38,             # (not actually used) set backlight on/off 
                "pins_data":        [28, 26, 24, 22]   # list of data GPIO pins
            },
        }
        print("Making CharLCD")
        return CharLCD(
            **__LCDMapping["GPIO"],
            cols=16, rows=2, dotsize=8, charmap='A02',
            auto_linebreaks=True, backlight_enabled=True,
        )

