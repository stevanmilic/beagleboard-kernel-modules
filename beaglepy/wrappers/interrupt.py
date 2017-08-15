"""Module for attaching interrupt on python pid function
wtih gpio pins on Beagleboard Black
"""

import threading
from .gpio import Gpio, INPUT, Device, config


class Interrupt(Gpio):

    """Wrapper class for interrupt handling, because interrupt
    is attached to gpio pin, Gpio class is inherited
    """

    def __init__(self, pin):
        super(Interrupt, self).__init__(pin, INPUT)
        self.attached = False
        self.mask = self.gpio_id

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
        while True:
            Device.poll(config.GPIO_DEVICE, self.mask)

            interrupt_handler(*args)
