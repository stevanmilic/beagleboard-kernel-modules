"""Module for interacting with gpio char device"""

from .singleton import Singleton
from .device import Device, os
from .config import GPIO_DEVICE


@Singleton
class GpioDevice(Device):

    """Interface for reading and writing to gpio char device"""

    def __init__(self):
        self.file_descriptor = os.open(GPIO_DEVICE, os.O_RDWR)

    def get_fd(self):
        """Method to get file descriptor"""
        return self.file_descriptor
