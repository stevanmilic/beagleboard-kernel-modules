"""Module for interacting with pwm pins on Beagleboard Black"""

import os
import config
from device import Device

NANO_SECOND = float(10**9)


class Pwm(object):

    """Wrapper class arround kernel module for pwm pins"""

    def __init__(self, pin):
        self._config_pin_to_pwm(pin)
        self.pwm_id = self._get_pwm_chip_id(pin)
        command = '{} {}'.format(
            config.INIT_OPTION, self.pwm_id)
        Device.write(config.PWM_DEVICE, command)

    def read(self):
        """"Method to read duty_cycle and period from pwm pin"""
        command = "{} {}".format(config.READ_OPTION, self.pwm_id)
        Device.write(config.PWM_DEVICE, command)
        return Device.read(config.PWM_DEVICE, 32)

    def write(self, duty_percent, frequency):
        """Method to write duty_percent and frequency to pwm pin,
        pwm chips read period(frequency) and duty_cycle(duty_percent) in
        nano seconds, so conversion is needed
        """
        period = int(round(NANO_SECOND / frequency))
        duty_cycle = int(round(period * duty_percent))
        command = '{} {} {} {}'.format(
            config.WRITE_OPTION, self.pwm_id, duty_cycle, period)
        Device.write(config.PWM_DEVICE, command)

    def free(self):
        """Method to free pwm pin from module using it"""
        command = "{} {}".format(config.FREE_OPTION, self.pwm_id)
        Device.write(config.PWM_DEVICE, command)

    @staticmethod
    def _config_pin_to_pwm(pin):
        """Function for configuring pin to pwm state, for achieving this
        we are using config-pin program which is included in newer kernels
        """
        command = 'config-pin {} pwm'.format(pin)
        output = os.system(command)
        if output is not 0:
            print 'Error while running config-pin, exiting...'
            exit(output)

    @staticmethod
    def _get_pwm_chip_id(pin):
        """Function serves to get pwm chip id which is trouved in sys path,
        ids for this chip are reseted each time bbb is powered up.
        Every pwm pin has it's own chip and address indetification, and also
        his index, with this params we can find our pwm chip.
        """
        pwm_chip_path = '/sys/devices/platform/ocp/{}.epwmss/{}.pwm/pwm'.format(
            config.PINS[pin]['pwm']['chip'], config.PINS[pin]['pwm']['addr'])
        for pwm_chip_file in os.listdir(pwm_chip_path):
            if pwm_chip_file.startswith('pwmchip'):
                return int(pwm_chip_file[-1:]) + config.PINS[pin]['pwm']['index']
