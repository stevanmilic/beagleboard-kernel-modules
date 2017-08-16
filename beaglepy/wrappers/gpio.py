"""Module for interacting with gpio pins on Beagleboard Black"""

from .gpio_device import GpioDevice
from . import config

INPUT = 0
OUTPUT = 1

LOW = 0
HIGH = 1


class Gpio(object):

    """Wrapper class arround kernel module for gpio pins"""

    def __init__(self, pin, direction):
        self._gpio_id = config.PINS[pin]['gpio']
        self.state = LOW
        self.device = GpioDevice.instance()
        command = self.get_init_command(direction)
        self.device.write(command)

    def get_init_command(self, direction):
        """Method for producing command for init the device"""
        return "{} {} {} {}".format(config.INIT_OPTION, self._gpio_id, direction, 0)

    def read(self):
        """"Method to read a value from gpio pin"""
        command = "{} {}".format(config.READ_OPTION, self._gpio_id)
        self.device.write(command)
        read = self.device.read(4)

        if read == '0':
            self.state = LOW
        elif read == '1':
            self.state = HIGH

        return self.state

    def write(self, value):
        """Method to write a value to gpio pin"""
        command = "{} {} {}".format(config.WRITE_OPTION, self._gpio_id, value)
        self.device.write(command)
        self.state = value

    def free(self):
        """Method to free gpio pin from module using it"""
        command = "{} {}".format(config.FREE_OPTION, self._gpio_id)
        self.device.write(command)
