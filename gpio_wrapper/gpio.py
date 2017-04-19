"""Module for interacting with gpio pins on Beagleboard Black"""

from pin import Pin
from device import Device


class Gpio(object):

    """Wrapper class arround kernel module for gpio pins"""

    initOption = 'i'
    readOption = 'r'
    writeOption = 'w'
    freeOption = 'f'
    gpioDevice = 'bbgpio'

    def __init__(self, pin, direction):
        # self.id = get_gpioId(pin)
        self.gpioId = 26
        command = "{} {} {}".format(self.initOption, self.gpioId, direction)
        Device.write(self.gpioDevice, command)

    def read(self):
        """"Method to read a value from gpio pin"""
        command = "{} {}".format(self.readOption, self.gpioId)
        Device.write(self.gpioDevice, command)
        return Device.read(self.gpioDevice, 1)

    def write(self, value):
        """Method to write a value to gpio pin"""
        command = "{} {} {}".format(self.writeOption, self.gpioId, value)
        Device.write(self.gpioDevice, command)

    def free(self, pin):
        """Method to free gpio pin from module using it"""
        command = "{} {}".format(self.freeOption, self.gpioId)
        Device.write(self.gpioDevice, command)
