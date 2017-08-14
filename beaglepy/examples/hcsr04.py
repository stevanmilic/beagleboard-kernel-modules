"""Module for using hc-sr04 ultrasonic sensor with gpio pins on the bbb"""

from time import sleep, time
from beaglepy.wrappers.gpio import Gpio, OUTPUT, INPUT, LOW, HIGH

TRIGGER_PIN = 'P8_14'
ECHO_PIN = 'P8_13'

MAX_RANGE = 400  # cm
MICRO_SECOND = 10**(-6)


def main():
    """Main function where we program and receive signals from sensor"""

    trigger_gpio = Gpio(TRIGGER_PIN, OUTPUT)
    echo_gpio = Gpio(ECHO_PIN, INPUT)

    trigger_pulse(trigger_gpio)
    pulse_duration = echo_pulse(echo_gpio)

    distance = range_cm(pulse_duration)
    if distance > MAX_RANGE:
        print 'Out of range'
    else:
        print str(distance) + ' cm'

    trigger_gpio.free()
    echo_gpio.free()


def trigger_pulse(trigger_gpio):
    """Function for supplying a short 10uS pulse to the trigger gpio  to start
    the ranging, and then module will send out an 8 cycle burst of ultrasound
    at 40kHz and raise its echo
    """
    trigger_gpio.write(LOW)
    delay_microseconds(2)
    trigger_gpio.write(HIGH)
    delay_microseconds(10)
    trigger_gpio.write(LOW)


def echo_pulse(echo_gpio):
    """Function for reading echo gpio which is supposed to have High voltage
    and by that getting the required parametar for calculating the distance
    of the object
    """
    while echo_gpio.read() is LOW:
        pass

    start = time()
    while echo_gpio.read() is HIGH:
        pass
    end = time()

    return (end - start)/MICRO_SECOND


def range_cm(pulse_duration):
    """Function for calculating distance using the pulse duration from
    the echo gpio, the formula can be found in sensor cheatsheat.
    The formula is connected to the speed of sound in the air
    """
    return pulse_duration/58.0


def delay_microseconds(delay):
    """A help function for sleeping a thread in microseconds"""
    sleep(delay*MICRO_SECOND)


if __name__ == "__main__":
    main()
