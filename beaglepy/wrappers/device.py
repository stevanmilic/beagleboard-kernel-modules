"""Module with interface for char device(driver)"""

from abc import ABCMeta, abstractmethod
import os
import select


class Device(object):
    """Abstract interface for char device"""

    __metaclass__ = ABCMeta

    def read(self, no_bytes):
        """Method for reading from self.get_fd()"""
        output = os.read(self.get_fd(), no_bytes)
        return output

    def write(self, command):
        """Method for writing to self.get_fd()"""
        output = os.write(self.get_fd(), command)
        return output

    def poll(self, eventmask):
        """Method for polling self.get_fd()"""
        poll = select.poll()
        poll.register(self.get_fd(), eventmask)
        poll.poll()
        poll.unregister(self.get_fd())

    def close(self):
        """Method to close file descriptor"""
        os.close(self.get_fd())

    @abstractmethod
    def get_fd(self):
        """Abstract method to get file descriptor"""
        raise NotImplementedError('Method \'get_fd\' must be implemented')
