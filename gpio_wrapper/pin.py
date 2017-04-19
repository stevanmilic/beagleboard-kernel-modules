"""Module for base class for interacting with Beagleboard black pins"""


class Pin(object):

    """Wrapper base class arround kernel module for bbb pins"""

    def read(self):
        """Method to read a message from device"""
        raise NotImplementedError("Please Implement this method")

    def write(self):
        """Method to read from device"""
        raise NotImplementedError("Please Implement this method")

    def free(self):
        """Method to free device"""
        raise NotImplementedError("Please Implement this method")
