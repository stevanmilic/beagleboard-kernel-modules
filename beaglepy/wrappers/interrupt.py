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
        self.attached = False
        super(Interrupt, self).__init__(pin, INPUT)

    def attach_interrupt(self, interrupt_handler, *args):
        """Method to attach interrupt on handler passed as reference,
        local method is used to detect signal from device
        """

        self.attached = True

        def handler(signum, frame):
            """Local method to call handler passed by user
            for specific gpio pin"""
            print 'Signal handler called with signal', signum
            interrupt_handler(*args)

        signal.signal(signal.SIGIO, handler)
