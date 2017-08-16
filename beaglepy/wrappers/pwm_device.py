"""Module for interacting with pwm char device"""

from .singleton import Singleton
from .device import Device, os
from .config import PWM_DEVICE


@Singleton
class PwmDevice(Device):

    """Interface for reading and writing to gpio char device"""

    def __init__(self):
        self.file_descriptor = os.open(PWM_DEVICE, os.O_RDWR)

    def get_fd(self):
        """Method to get file descriptor"""
        return self.file_descriptor
