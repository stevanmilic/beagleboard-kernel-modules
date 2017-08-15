"""Module for interacting with linux device"""

import os
import select


class Device(object):

    """Interface for reading and writing to device"""

    @staticmethod
    def read(name, no_bytes):
        """Method for reading from device"""
        device = os.open(Device._path(name), os.O_RDONLY)
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
    def poll(name, eventmask):
        """Method for polling device"""
        device = os.open(Device._path(name), os.O_RDONLY)
        poll = select.poll()
        poll.register(device, eventmask)
        poll.poll()
        poll.unregister(device)
        os.close(device)

    @staticmethod
    def _path(name):
        return '/dev/' + name
