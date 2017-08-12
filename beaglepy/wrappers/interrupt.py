"""Module for attaching interrupt on python pid function
wtih gpio pins on Beagleboard Black
"""

import os
import signal
from .gpio import Gpio, INPUT


class Interrupt(Gpio):

    """Wrapper class for interrupt handling, because interrupt
    is attached to gpio pin, Gpio class is inherited
    """

    def __init__(self, pin):
        self.pid = os.getpid()
        super(Interrupt, self).__init__(pin, INPUT)

    def attach_interrupt(self, interrupt_handler):
        """Method to attach interrupt on handler passed as reference,
        local method is used to detect signal from device
        """

        def handler(signum, frame):
            """Local method to call handler passed by user
            for specific gpio pin"""
            print 'Signal handler called with signal', signum
            # if frame.id == self.gpio_id:
            interrupt_handler()

        signal.signal(signal.SIGIO, handler)
