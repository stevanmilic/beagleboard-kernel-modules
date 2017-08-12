"""Module for interacting with gpio pins on Beagleboard Black"""

from .device import Device
from . import config

INPUT = 0
OUTPUT = 1

LOW = 0
HIGH = 1


class Gpio(object):

    """Wrapper class arround kernel module for gpio pins"""

    pid = 0

    def __init__(self, pin, direction):
        self.gpio_id = config.PINS[pin]['gpio']
        command = "{} {} {} {}".format(
            config.INIT_OPTION, self.gpio_id, direction, self.pid)
        Device.write(config.GPIO_DEVICE, command)

    def read(self):
        """"Method to read a value from gpio pin"""
        command = "{} {}".format(config.READ_OPTION, self.gpio_id)
        Device.write(config.GPIO_DEVICE, command)
        return Device.read(config.GPIO_DEVICE, 4)

    def write(self, value):
        """Method to write a value to gpio pin"""
        command = "{} {} {}".format(config.WRITE_OPTION, self.gpio_id, value)
        Device.write(config.GPIO_DEVICE, command)

    def free(self):
        """Method to free gpio pin from module using it"""
        command = "{} {}".format(config.FREE_OPTION, self.gpio_id)
        Device.write(config.GPIO_DEVICE, command)
