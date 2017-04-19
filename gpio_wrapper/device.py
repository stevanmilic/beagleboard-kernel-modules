"""Module for interacting with linux device"""

import os


class Device(object):

    """Interface for reading and writing to device"""

    @staticmethod
    def read(name, no_bytes):
        """Method for reading from device"""
        device = os.open(Device._path(name), os.O_RDONLY)
        # os.lseek(device, 0, os.SEEK_SET)
        output = os.read(device, no_bytes)
        os.close(device)
        return output

    @staticmethod
    def write(name, command):
        """Method for writing to device"""
        device = os.open(Device._path(name), os.O_WRONLY)
        output = os.write(device, command)
        os.close(device)
        return output

    @staticmethod
    def _path(name):
        return '/dev/' + name
