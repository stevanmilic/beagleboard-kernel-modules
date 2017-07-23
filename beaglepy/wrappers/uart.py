"""Module for interacting with uart pins on Beagleboard Black"""

import os
import serial
from .config import INDEXED_PINS


class Uart(object):

    """Wrapper class arround serial module for uart pins"""

    def __init__(self, uart, baudrate):
        pins = self._get_uart_pins(uart)
        for pin in pins:
            self._config_pin_to_uart(pin)
        self.ser = serial.Serial('/dev/ttyO' + uart[-1:], baudrate)
        if self.ser.isOpen():
            self.ser.close()
        self.ser.open()

    def read(self):
        """Method for reading serial uart port"""
        return self.ser.readline().strip('\r\n')

    def write(self, command):
        """Method for writing command to serial uart port"""
        self.ser.flushInput()
        self.ser.write(command + '\r\n')

    def free(self):
        """Method to free serial port from using it"""
        self.ser.close()

    def is_busy(self):
        """Method to check if serial port is busy"""
        return self.ser.inWaiting()

    @staticmethod
    def _get_uart_pins(uart):
        """Function for getting the according uart rx and tx pins"""
        uart = uart.upper()
        for indexed_pin in INDEXED_PINS:
            if indexed_pin['name'] == uart + '_TXD':
                tx_pin = indexed_pin['key']
            elif indexed_pin['name'] == uart + '_RXD':
                rx_pin = indexed_pin['key']
        return (tx_pin, rx_pin)

    @staticmethod
    def _config_pin_to_uart(pin):
        """Function for configuring pin to uart state, for achieving this
        we are using config-pin program which is included in newer kernels
        """
        command = 'config-pin {} uart'.format(pin)
        output = os.system(command)
        if output is not 0:
            print 'Error while running config-pin, exiting...'
            exit(output)
