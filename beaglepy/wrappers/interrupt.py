"""Module for attaching interrupt on python pid function
wtih gpio pins on Beagleboard Black
"""

import threading
import itertools
from .gpio import Gpio, INPUT, config


class Interrupt(Gpio):

    """Wrapper class for interrupt handling, because interrupt
    is attached to gpio pin, Gpio class is inherited
    """

    posid = itertools.count().next

    def __init__(self, pin):
        self.attached = False
        self._mask = 1 << Interrupt.posid()
        super(Interrupt, self).__init__(pin, INPUT)

    def get_init_command(self, direction):
        """Method for producing command for init the device"""
        return "{} {} {} {}".format(config.INIT_OPTION, self._gpio_id, direction, self._mask)

    def attach_interrupt(self, interrupt_handler, *args):
        """Method to attach interrupt on handler passed as reference,
        local method is used to detect signal from device
        """

        self.attached = True

        thread = threading.Thread(
            target=self.poll_gpio, args=(interrupt_handler, args))
        thread.deamon = True
        thread.start()

    def poll_gpio(self, interrupt_handler, args):
        """Method to poll device until change on gpio is happened,
        and thus triggering attached handler function
        """
        while self.attached:
            self.device.poll(self._mask)

            if self.attached:
                interrupt_handler(*args)

    def free(self):
        """Override base class method to break polling"""
        self.attached = False
        super(Interrupt, self).free()
